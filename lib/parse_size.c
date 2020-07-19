/* SPDX-License-Identifier: ISC */
/*
 * parse_size.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "gcfg.h"

const char *gcfg_parse_size(gcfg_file_t *f, const char *in, uint64_t *ret)
{
	const char *start = in;
	uint64_t shift;

	in = gcfg_dec_num(f, in, ret, 0xFFFFFFFFFFFFFFFF);
	if (in == NULL)
		return NULL;

	switch (*in) {
	case 'k':
	case 'K':
		shift = 10;
		break;
	case 'm':
	case 'M':
		shift = 20;
		break;
	case 'g':
	case 'G':
		shift = 30;
		break;
	case 't':
	case 'T':
		shift = 40;
		break;
	default:
		shift = 0;
		break;
	}

	if ((*ret) > (0xFFFFFFFFFFFFFFFFUL >> shift)) {
		f->report_error(f, "numeric overflow in %.6s...", start);
		return NULL;
	}

	(*ret) <<= shift;
	return in;
}

#ifdef BUILD_TEST
static const struct {
	const char *in;
	uint64_t out;
	int ret;
} testvec[] = {
	{ "100",       100, 0 },
	{ "100k",      102400, 0 },
	{ "100K",      102400, 0 },
	{ "100m",      104857600UL, 0 },
	{ "100M",      104857600UL, 0 },
	{ "16777215T", 0xFFFFFF0000000000UL, 0 },
	{ "16777216T", 0, -1 },
	{ "-100k",     0, -1 },
	{ "k",         0, -1 },
	{ "foobar",    0, -1 },
};

static void test_case(gcfg_file_t *df, size_t i)
{
	const char *ret;
	uint64_t out;

	ret = gcfg_parse_size(df, testvec[i].in, &out);

	if ((ret == NULL && testvec[i].ret == 0) ||
	    (ret != NULL && testvec[i].ret != 0)) {
		fprintf(stderr, "Wrong return status for %zu\n", i);
		fprintf(stderr, "Input: '%s' was %s\n",
			testvec[i].in,
			ret == NULL ? "not accepted" : "accepted");
		exit(EXIT_FAILURE);
	}

	if (ret == NULL)
		return;

	if (out != testvec[i].out) {
		fprintf(stderr, "Mismatch for %zu\n", i);
		fprintf(stderr, "Expected: %lu\n",
			(unsigned long)testvec[i].out);
		fprintf(stderr, "Received: %lu\n",
			(unsigned long)out);
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
