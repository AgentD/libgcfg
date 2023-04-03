/* SPDX-License-Identifier: ISC */
/*
 * parse_number.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"
#include "test.h"

#define NUMBER(a, b) \
	{ .data = { .number = {{a, b}} }, .type=GCFG_VALUE_NUMBER }

#define PERCENTAGE(a, b) \
	{ .data = { .number = {{a, b}} }, .type=GCFG_VALUE_PERCENTAGE }

static const struct {
	const char *in;
	gcfg_value_t out;
	double dbl;
	int ret;
} testvec[] = {
	{ "", .ret = -1 },
	{ "+", .ret = -1 },
	{ "-", .ret = -1 },
	{ "13", NUMBER(13, 0), 13.0, 0 },
	{ "+13", NUMBER(13, 0), 13.0, 0 },
	{ "-13", NUMBER(-13, 0), -13.0, 0 },
	{ "13.37", NUMBER(1337, -2), 13.37, 0 },
	{ "+13.37", NUMBER(1337, -2), 13.37, 0 },
	{ "-13.37", NUMBER(-1337, -2), -13.37, 0 },
	{ "13%", PERCENTAGE(13, -2), .13, 0 },
	{ "+13%", PERCENTAGE(13, -2), .13, 0 },
	{ "-13%", PERCENTAGE(-13, -2), -.13, 0 },
	{ "13.37%", PERCENTAGE(1337, -4), .1337, 0 },
	{ "+13.37%", PERCENTAGE(1337, -4), .1337, 0 },
	{ "-13.37%", PERCENTAGE(-1337, -4), -.1337, 0 },
	{ "13e-42", NUMBER(13,-42), 13e-42, 0 },
	{ "+13e+42", NUMBER(13, 42), 13e42, 0 },
	{ "-13e-42", NUMBER(-13, -42), -13e-42, 0 },
	{ "13.37e-42", NUMBER(1337, -44), 13.37e-42, 0 },
	{ "+13.37e42", NUMBER(1337, 40), 13.37e42, 0 },
	{ "-13.37e-42", NUMBER(-1337, -44), -13.37e-42, 0 },
};

static void print_num(char *buffer, const gcfg_value_t *out)
{
	sprintf(buffer, "%ld * 10^%d",
		(long)out->data.number[0].value, out->data.number[0].exponent);
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
	gcfg_value_t out;
	char buffer[128];
	const char *ret;
	double dbl;

	memset(&out, 0, sizeof(out));

	ret = gcfg_parse_number(df, testvec[i].in, &out, 0);

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

	dbl = gcfg_number_to_double(&out, 0);

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
