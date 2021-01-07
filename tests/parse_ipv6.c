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
