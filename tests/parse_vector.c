/* SPDX-License-Identifier: ISC */
/*
 * parse_vector.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"
#include "test.h"

#define VEC1(a, b) \
	{ .data = { .number = {{a, b}} }, .type = GCFG_VALUE_NUMBER }

#define VEC2(a, b, c, d) \
	{ .data = { .number = {{a, b}, {c, d}} }, .type = GCFG_VALUE_VEC2 }

#define VEC3(a, b, c, d, e, f) \
	{ .data = { .number = {{a, b}, {c, d}, {e, f}} }, \
	  .type = GCFG_VALUE_VEC3 }

#define VEC4(a, b, c, d, e, f, g, h) \
	{ .data = { .number = {{a, b}, {c, d}, {e, f}, {g, h}} }, \
	  .type = GCFG_VALUE_VEC4 }

static const struct {
	const char *in;
	size_t count;
	int ret;
	gcfg_value_t out;
} testvec[] = {
	{ "(+1,-2,+3,-4)", 4, 0, VEC4(1, 0, -2, 0, 3, 0, -4, 0) },
	{ "(+1,-2,+3)", 3, 0, VEC3(1, 0, -2, 0, 3, 0) },
	{ "(+1,-2)", 2, 0, VEC2(1, 0, -2, 0) },
	{ "(-2)", 1, 0, VEC1(-2, 0) },
	{ "(+1,-2,+3,-4)", 3, -1, VEC1(0,0) },
	{ "(+1,-2,+3)", 4, -1, VEC1(0,0) },
};

static void test_case(gcfg_file_t *df, size_t i)
{
	gcfg_value_t out;
	const char *ret;

	memset(&out, 0, sizeof(out));

	ret = gcfg_parse_vector(df, testvec[i].in, &out, testvec[i].count);

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

	if (memcmp(&out, &(testvec[i].out), sizeof(out))) {
		fprintf(stderr, "Mismatch for %zu\n", i);
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
