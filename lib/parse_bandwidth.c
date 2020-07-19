/* SPDX-License-Identifier: ISC */
/*
 * parse_bandwidth.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "gcfg.h"

#define PREFIX(bin, dec) \
		++in; \
		if (*in == 'i' || *in == 'I') { \
			++in; scale = (bin); \
		} else { \
			scale = (dec); \
		}

const char *gcfg_parse_bandwidth(gcfg_file_t *f, const char *in, uint64_t *ret)
{
	const char *start = in;
	uint64_t scale;

	in = gcfg_dec_num(f, in, ret, 0xFFFFFFFFFFFFFFFF);
	if (in == NULL)
		return NULL;

	switch (*in) {
	case 'k':
	case 'K':
		PREFIX(1UL << 10UL, 1000UL)
		break;
	case 'm':
	case 'M':
		PREFIX(1UL << 20UL, 1000000UL)
		break;
	case 'g':
	case 'G':
		PREFIX(1UL << 30UL, 1000000000UL)
		break;
	case 't':
	case 'T':
		PREFIX(1UL << 40UL, 1000000000000UL);
		break;
	default:
		scale = 1;
		break;
	}

	if ((in[0] == 'b' || in[0] == 'B') && (in[1] == 'p' || in[1] == 'P') &&
	    (in[2] == 's' || in[2] == 'S')) {
		in += 3;
		scale *= 8;
	} else if ((in[0] == 'b' || in[0] == 'B') && (in[1] == 'i' || in[1] == 'I') &&
		   (in[2] == 't' || in[2] == 'T')) {
		in += 3;
	} else if (*in == 'B') {
		++in;
		scale *= 8;
	} else if (*in == 'b') {
		++in;
	}

	if ((*ret) > (0xFFFFFFFFFFFFFFFFUL / scale)) {
		f->report_error(f, "numeric overflow in %.6s...", start);
		return NULL;
	}

	(*ret) *= scale;
	return in;
}

#ifdef BUILD_TEST
static const struct {
	const char *in;
	uint64_t out;
	int ret;
} testvec[] = {
	{ "100",      100, 0 },
	{ "100b",     100, 0 },
	{ "100B",     800, 0 },
	{ "100bit",   100, 0 },
	{ "100Bit",   100, 0 },
	{ "100bps",   800, 0 },
	{ "100k",     100000, 0 },
	{ "100kb",    100000, 0 },
	{ "100kB",    800000, 0 },
	{ "100kbit",  100000, 0 },
	{ "100kbps",  800000, 0 },
	{ "100ki",    102400, 0 },
	{ "100kib",   102400, 0 },
	{ "100kiB",   819200, 0 },
	{ "100kibit", 102400, 0 },
	{ "100kibps", 819200, 0 },
	{ "100m",     100000000UL, 0 },
	{ "100mb",    100000000UL, 0 },
	{ "100mB",    800000000UL, 0 },
	{ "100mbit",  100000000UL, 0 },
	{ "100mbps",  800000000UL, 0 },
	{ "100mi",    104857600UL, 0 },
	{ "100mib",   104857600UL, 0 },
	{ "100miB",   838860800UL, 0 },
	{ "100mibit", 104857600UL, 0 },
	{ "100mibps", 838860800UL, 0 },
	{ "-100k", 0, -1 },
	{ "kbit", 0, -1 },
	{ "foobar", 0, -1 },
};

static void test_case(gcfg_file_t *df, size_t i)
{
	const char *ret;
	uint64_t out;

	ret = gcfg_parse_bandwidth(df, testvec[i].in, &out);

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
