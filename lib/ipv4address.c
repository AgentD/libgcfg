/* SPDX-License-Identifier: ISC */
/*
 * ipv4address.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"

const char *gcfg_ipv4address(gcfg_file_t *f, const char *str, uint32_t *out)
{
	const char *start = str;
	uint64_t temp;
	int i;

	*out = 0;

	for (i = 0; i < 4; ++i) {
		if (i > 0 && *(str++) != '.')
			goto fail;

		str = gcfg_dec_num(f, str, &temp, 255);
		if (str == NULL)
			return NULL;

		*out = ((*out) << 8) | ((uint32_t)(temp & 0x00FF));
	}
	return str;
fail:
	if (f != NULL)
		f->report_error(f, "malformed IPv4 address '%.5s...'", start);
	return NULL;
}

#ifdef BUILD_TEST
static const struct {
	const char *in;
	uint32_t out;
	int ret;
} testvec[] = {
	{ "192.168.0.1", 0xC0A80001, 0 },
	{ "192.168.0", 0, -1 },
	{ "192.168", 0, -1 },
	{ "192", 0, -1 },
	{ "foo", 0, -1 },
	{ "", 0, -1 },
	{ "192.168.0.1foobar", 0xC0A80001, 0 },
	{ "259.512.892.42", 0, -1 },
};

static void print_ip(char *buffer, uint32_t ip)
{
	sprintf(buffer, "%d.%d.%d.%d",
		(int)((ip >> 24) & 0x00FF), (int)((ip >> 16) & 0x00FF),
		(int)((ip >> 8) & 0x00FF), (int)(ip & 0x00FF));
}

static void test_case(gcfg_file_t *df, size_t i)
{
	char buffer[64];
	const char *ret;
	uint32_t ipout;

	ret = gcfg_ipv4address(df, testvec[i].in, &ipout);

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

	if (ipout != testvec[i].out) {
		fprintf(stderr, "Mismatch for %zu\n", i);
		print_ip(buffer, testvec[i].out);
		fprintf(stderr, "Expected: %s\n", buffer);
		print_ip(buffer, ipout);
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
