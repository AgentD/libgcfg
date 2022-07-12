/* SPDX-License-Identifier: ISC */
/*
 * number_to_double.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"
#include "test.h"

#include <math.h>

static const struct {
	gcfg_number_t input;
	int pad0[3];
	double result;
} test_vec[] = {
	{ .input = { 13, 37, 0, 2, 0 }, .result = 13.37 },
	{ .input = { 13, 37, 0, 2, GCFG_NUM_NEGATIVE }, .result = -13.37 },
	{ .input = { 13, 37, 2, 2, 0 }, .result = 1337.0 },
	{ .input = { 13, 37, 2, 2, GCFG_NUM_NEGATIVE }, .result = -1337.0 },
};

int main(void)
{
	double diff, expected, ret;
	size_t i;

	for (i = 0; i < sizeof(test_vec) / sizeof(test_vec[0]); ++i) {
		expected = test_vec[i].result;

		ret = gcfg_number_to_double(&test_vec[i].input);

		diff = fabs(expected - ret);

		if (diff > 1e-15) {
			fprintf(stderr, "'%c%u.%0*ue%d' was parsed as %f!\n",
				(test_vec[i].input.flags & GCFG_NUM_NEGATIVE) ? '-' : '+',
				test_vec[i].input.integer,
				(int)test_vec[i].input.fraction_digits,
				test_vec[i].input.fraction,
				test_vec[i].input.exponent,
				ret);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
