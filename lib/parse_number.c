/* SPDX-License-Identifier: ISC */
/*
 * parse_number.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "gcfg.h"
#include "util.h"

const char *gcfg_parse_number(gcfg_file_t *f, const char *in,
			      gcfg_number_t *num)
{
	uint64_t temp;
	int i;

	num->sign = (*in == '-') ? -1 : 1;
	if (*in == '+' || *in == '-')
		++in;

	in = gcfg_dec_num(f, in, &temp, 0xFFFFFFFF);
	if (in == NULL)
		return NULL;

	num->integer = (uint32_t)temp;

	if (*in == '.') {
		++in;
		for (i = 0; in[i] >= '0' && in[i] <= '9'; ++i)
			;

		in = gcfg_dec_num(f, in, &temp, 0xFFFFFFFF);
		if (in == NULL)
			return NULL;

		num->fraction_digits = (uint16_t)i;
		num->fraction = (uint32_t)temp;
	} else {
		num->fraction_digits = 0;
		num->fraction = 0;
	}

	switch (*in) {
	case 'e':
	case 'E':
		i = (in[1] == '-') ? -1 : 1;
		in += (in[1] == '-' || in[1] == '+') ? 2 : 1;

		in = gcfg_dec_num(f, in, &temp, 0x7FFF);
		if (in == NULL)
			return NULL;

		num->exponent = (int16_t)((int)temp * i);
		break;
	case '%':
		++in;
		num->exponent = -2;
		break;
	default:
		num->exponent = 0;
		break;
	}

	return in;
}

#ifdef BUILD_TEST
static const struct {
	const char *in;
	gcfg_number_t out;
	double dbl;
	int ret;
} testvec[] = {
	{ "", .ret = -1 },
	{ "+", .ret = -1 },
	{ "-", .ret = -1 },
	{ "13", {1,13,0,0,0}, 13.0, 0 },
	{ "+13", {1,13,0,0,0}, 13.0, 0 },
	{ "-13", {-1,13,0,0,0}, -13.0, 0 },
	{ "13.37", {1,13,37,2,0}, 13.37, 0 },
	{ "+13.37", {1,13,37,2,0}, 13.37, 0 },
	{ "-13.37", {-1,13,37,2,0}, -13.37, 0 },
	{ "13%", {1,13,0,0,-2}, .13, 0 },
	{ "+13%", {1,13,0,0,-2}, .13, 0 },
	{ "-13%", {-1,13,0,0,-2}, -.13, 0 },
	{ "13.37%", {1,13,37,2,-2}, .1337, 0 },
	{ "+13.37%", {1,13,37,2,-2}, .1337, 0 },
	{ "-13.37%", {-1,13,37,2,-2}, -.1337, 0 },
	{ "13e-42", {1,13,0,0,-42}, 13e-42, 0 },
	{ "+13e+42", {1,13,0,0,42}, 13e42, 0 },
	{ "-13e-42", {-1,13,0,0,-42}, -13e-42, 0 },
	{ "13.37e-42", {1,13,37,2,-42}, 13.37e-42, 0 },
	{ "+13.37e42", {1,13,37,2,42}, 13.37e42, 0 },
	{ "-13.37e-42", {-1,13,37,2,-42}, -13.37e-42, 0 },
};

static void print_num(char *buffer, const gcfg_number_t *out)
{
	sprintf(buffer, "%c%u.%u * 10^%d",
		out->sign > 0 ? '+' : '-',
		out->integer, out->fraction, out->exponent);
}

static int dbl_cmp(double a, double b)
{
	double diff = a - b;

	if (diff < -1e-9)
		return -1;

	if (diff > 1e-9)
		return 1;

	return 0;
}

static void test_case(gcfg_file_t *df, size_t i)
{
	gcfg_number_t out;
	char buffer[128];
	const char *ret;
	double dbl;

	ret = gcfg_parse_number(df, testvec[i].in, &out);

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

	if (memcmp(&out, &testvec[i].out, sizeof(out)) != 0) {
		fprintf(stderr, "Mismatch for %zu\n", i);
		print_num(buffer, &testvec[i].out);
		fprintf(stderr, "Expected: %s\n", buffer);
		print_num(buffer, &out);
		fprintf(stderr, "Received: %s\n", buffer);
		exit(EXIT_FAILURE);
	}

	dbl = gcfg_number_to_double(&out);

	if (dbl_cmp(testvec[i].dbl, dbl) != 0) {
		fprintf(stderr, "Conversion mismatch for %zu\n", i);
		fprintf(stderr, "Expected: %f\n", testvec[i].dbl);
		fprintf(stderr, "Received: %f\n", dbl);
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
