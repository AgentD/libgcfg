/* SPDX-License-Identifier: ISC */
/*
 * parse_ipv6.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "gcfg.h"
#include "util.h"

static const char *h16(gcfg_file_t *f, const char *str, uint16_t *out)
{
	int i;

	if (gcfg_xdigit(*str) < 0)
		goto fail_num;

	*out = 0;

	for (i = 0; i < 4 && gcfg_xdigit(str[i]) >= 0; ++i)
		*out = (uint16_t)(*out << 4) | (uint16_t)gcfg_xdigit(str[i]);

	return str + i;
fail_num:
	if (f != NULL) {
		f->report_error(f, "expected hex digit instead of '%.6s...",
				str);
	}
	return NULL;
}

static const char *ls32(const char *str, uint16_t *hi, uint16_t *lo)
{
	const char *temp;
	uint32_t ipv4;

	temp = gcfg_ipv4address(NULL, str, &ipv4);
	if (temp != NULL) {
		*hi = (uint16_t)((ipv4 >> 16) & 0x0000FFFF);
		*lo = (uint16_t)( ipv4        & 0x0000FFFF);
		return temp;
	}

	str = h16(NULL, str, hi);
	if (str == NULL)
		return NULL;

	return *str == ':' ? h16(NULL, str + 1, lo) : NULL;
}

const char *gcfg_parse_ipv6(gcfg_file_t *f, const char *str,
			    gcfg_ip_addr_t *ret)
{
	uint16_t v[8], w[8];
	const char *temp;
	uint64_t mask;
	int i, j;

	memset(v, 0, sizeof(v));

	for (i = 0; i < 8; ++i) {
		if (str[0] == ':' && str[1] == ':') {
			str += 2;
			break;
		}

		if (i > 0) {
			if (*str != ':')
				goto fail_colon;
			++str;
		}

		if (i == 6) {
			temp = ls32(str, v + i, v + i + 1);
			if (temp != NULL) {
				str = temp;
				i += 2;
				break;
			}
		}

		str = h16(f, str, v + i);
		if (str == NULL)
			return NULL;
	}

	if (i < 8) {
		for (j = 0; j < 8; ++j) {
			if (*str == ' ' || *str == '\t' ||
			    *str == '/' || *str == '\0') {
				break;
			}
			if (j > 0) {
				if (*str != ':')
					goto fail_colon;
				++str;
			}

			temp = ls32(str, w + j, w + j + 1);
			if (temp != NULL &&
			    (*temp == ' ' || *temp == '\t' ||
			     *temp == '/' || *temp == '\0')) {
				j += 2;
				str = temp;
				break;
			}

			str = h16(f, str, w + j);
			if (str == NULL)
				return NULL;
		}

		if ((j + i) > 7) {
			f->report_error(f, "overlong IPv6 address");
			return NULL;
		}

		if (j > 0)
			memcpy(v + (8 - j), w, (size_t)j * sizeof(uint16_t));
	}

	if (*str == '/') {
		str = gcfg_dec_num(f, str + 1, &mask, 128);
		if (str == NULL)
			return NULL;

		ret->cidr_mask = (uint32_t)(mask & 0x00FF);
	} else {
		ret->cidr_mask = 128;
	}

	memcpy(ret->ip.v6, v, sizeof(v));
	return str;
fail_colon:
	f->report_error(f, "expected ':', found '%c'", *str);
	return NULL;
}

#ifdef BUILD_TEST
static const struct {
	const char *in;
	gcfg_ip_addr_t out;
	int ret;
} testvec[] = {
	{ "::", { { .v6 = {0,0,0,0,0,0,0,0} }, 128 }, 0 },
	{ "::/32",  { { .v6 = {0,0,0,0,0,0,0,0} }, 32 }, 0 },
	{ "::1", { { .v6 = {0,0,0,0,0,0,0,1} }, 128 }, 0 },
	{ "::1/32", { { .v6 = {0,0,0,0,0,0,0,1} }, 32 }, 0 },
	{ "::1/-1", .ret = -1 },
	{ "::1/130", .ret = -1 },
	{ "ffff::192.168.0.1/64", { { .v6 = {0xFFFF,0,0,0,0,0,0xC0A8,1} }, 64 }, 0 },
	{ "2001:41d0:52:cff::1714", { { .v6 = {0x2001,0x41D0,0x52,0xCFF,0,0,0,0x1714} }, 128 }, 0 },
	{ "ffff:::192.168.0.1/64", .ret = -1 },
};

static void print_ip(char *buffer, const gcfg_ip_addr_t *ip)
{
	sprintf(buffer, "%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X/%u",
		ip->ip.v6[0], ip->ip.v6[1], ip->ip.v6[2], ip->ip.v6[3],
		ip->ip.v6[4], ip->ip.v6[5], ip->ip.v6[6], ip->ip.v6[7],
		ip->cidr_mask);
}

static void test_case(gcfg_file_t *df, size_t i)
{
	gcfg_ip_addr_t ipout;
	char buffer[128];
	const char *ret;

	ret = gcfg_parse_ipv6(df, testvec[i].in, &ipout);

	if ((ret != NULL && testvec[i].ret != 0) ||
	    (ret == NULL && testvec[i].ret == 0)) {
		fprintf(stderr, "Wrong return status for %zu\n", i);
		fprintf(stderr, "Input: '%s' was %s\n",
			testvec[i].in,
			ret == NULL ? "not accepted" : "accepted");
		exit(EXIT_FAILURE);
	}

	if (ret == NULL)
		return;

	if (memcmp(ipout.ip.v6, testvec[i].out.ip.v6, 16) != 0 ||
	    ipout.cidr_mask != testvec[i].out.cidr_mask) {
		fprintf(stderr, "Mismatch for %zu\n", i);
		print_ip(buffer, &testvec[i].out);
		fprintf(stderr, "Expected: %s\n", buffer);
		print_ip(buffer, &ipout);
		fprintf(stderr, "Received: %s\n", buffer);
		exit(EXIT_FAILURE);
	}
}

int main(void)
{
	gcfg_file_t df;
	size_t i;

	for (i = 0; i < sizeof(testvec) / sizeof(testvec[0]); ++i) {
		dummy_file_init(&df, testvec[i].in);
		test_case(&df, i);
		dummy_file_cleanup(&df);
	}

	return EXIT_SUCCESS;
}
#endif
