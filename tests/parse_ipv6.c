/* SPDX-License-Identifier: ISC */
/*
 * parse_ipv6.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"
#include "test.h"

static const struct {
	const char *in;
	gcfg_value_t out;
	int ret;
} testvec[] = {
	{ "::", { { .ipv6 = {0,0,0,0,0,0,0,0} },
		  0, 128, GCFG_VALUE_IPV6 }, 0 },
	{ "::/32",  { { .ipv6 = {0,0,0,0,0,0,0,0} },
		  GCFG_NET_ADDR_HAVE_MASK, 32, GCFG_VALUE_IPV6 }, 0 },
	{ "::1", { { .ipv6 = {0,0,0,0,0,0,0,1} },
		  0, 128, GCFG_VALUE_IPV6 }, 0 },
	{ "::1/32", { { .ipv6 = {0,0,0,0,0,0,0,1} },
		  GCFG_NET_ADDR_HAVE_MASK, 32, GCFG_VALUE_IPV6 }, 0 },
	{ "::1/-1", .ret = -1 },
	{ "::1/130", .ret = -1 },
	{ "ffff::192.168.0.1/64", { { .ipv6 = {0xFFFF,0,0,0,0,0,0xC0A8,1} },
		  GCFG_NET_ADDR_HAVE_MASK, 64, GCFG_VALUE_IPV6 }, 0 },
	{ "2001:41d0:52:cff::1714", {
			{ .ipv6 = {0x2001,0x41D0,0x52,0xCFF,0,0,0,0x1714} },
			0, 128, GCFG_VALUE_IPV6 }, 0 },
	{ "ffff:::192.168.0.1/64", .ret = -1 },
};

static void print_ip(char *buffer, const gcfg_value_t *ip)
{
	sprintf(buffer, "%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X/%u",
		ip->data.ipv6[0], ip->data.ipv6[1],
		ip->data.ipv6[2], ip->data.ipv6[3],
		ip->data.ipv6[4], ip->data.ipv6[5],
		ip->data.ipv6[6], ip->data.ipv6[7],
		ip->cidr_mask);
}

static void test_case(gcfg_file_t *df, size_t i)
{
	gcfg_value_t ipout;
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

	if (ipout.type != testvec[i].out.type) {
		fprintf(stderr, "Mismatching type for %zu\n", i);
		fprintf(stderr, "Expected: %X\n", testvec[i].out.type);
		fprintf(stderr, "Received: %X\n", ipout.type);
		exit(EXIT_FAILURE);
	}

	if (ipout.flags != testvec[i].out.flags) {
		fprintf(stderr, "Mismatching flags for %zu\n", i);
		fprintf(stderr, "Expected: %X\n", testvec[i].out.flags);
		fprintf(stderr, "Received: %X\n", ipout.flags);
		exit(EXIT_FAILURE);
	}

	if (memcmp(ipout.data.ipv6, testvec[i].out.data.ipv6, 16) != 0 ||
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
