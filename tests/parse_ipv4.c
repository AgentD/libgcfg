/* SPDX-License-Identifier: ISC */
/*
 * parse_ipv4.c
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
	{ "192.168.0.1", { { .v4 = 0xC0A80001 }, 32 }, 0 },
	{ "192.168.0", .ret = -1 },
	{ "192.168", .ret = -1 },
	{ "192", .ret = -1 },
	{ "", .ret = -1 },
	{ "192.168.0.1/32", { { .v4 = 0xC0A80001 }, 32 }, 0 },
	{ "192.168.0.1/24", { { .v4 = 0xC0A80001 }, 24 }, 0 },
	{ "192.168.0.1/16", { { .v4 = 0xC0A80001 }, 16 }, 0 },
	{ "192.168.0.1/0", { { .v4 = 0xC0A80001 }, 0  }, 0 },
	{ "192.168.0.1/35", .ret = -1 },
	{ "192.168.0.1/-1", .ret = -1 },
	{ "192.168.0.1/foo", .ret = -1 },
	{ "192.168.0.1foo", .ret = -1 },
	{ "192.168.0/16", .ret = -1 },
	{ "192.168/16", .ret = -1 },
	{ "192/16", .ret = -1 },
	{ "/16", .ret = -1 },
	{ "Hello, world!", .ret = -1 },
	{ "259.512.892.42", .ret = -1 },
};

static void print_ip(char *buffer, const gcfg_ip_addr_t *ip)
{
	sprintf(buffer, "%d.%d.%d.%d/%u",
		(int)((ip->ip.v4 >> 24) & 0x00FF),
		(int)((ip->ip.v4 >> 16) & 0x00FF),
		(int)((ip->ip.v4 >> 8) & 0x00FF),
		(int)(ip->ip.v4 & 0x00FF),
		ip->cidr_mask);
}

static void test_case(gcfg_file_t *df, size_t i)
{
	gcfg_ip_addr_t ipout;
	char buffer[64];
	const char *ret;

	ret = gcfg_parse_ipv4(df, testvec[i].in, &ipout);

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

	if (ipout.ip.v4 != testvec[i].out.ip.v4 ||
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
