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
	{ .input = { 1337, 0, 0 }, .result = 1337.0 },
	{ .input = { -1337, 0, 0 }, .result = -1337.0 },
	{ .input = { 1337, -2, 0 }, .result = 13.37 },
	{ .input = { -1337, -2, 0 }, .result = -13.37 },
	{ .input = { 42, 2, 0 }, .result = 4200.0 },
	{ .input = { -42, 2, 0 }, .result = -4200.0 },
};

int main(void)
{
	double diff, expected, ret;
	size_t i;

	for (i = 0; i < sizeof(test_vec) / sizeof(test_vec[0]); ++i) {
		expected = test_vec[i].result;

		ret = gcfg_number_to_double(&test_vec[i].input);

		diff = fabs(expected - ret);

		if (diff > 1e-10) {
			fprintf(stderr, "'%lde%d' was converted "
				"to %f instead of %f (diff %f)!\n",
				(long)test_vec[i].input.value,
				test_vec[i].input.exponent,
				ret, expected, diff);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
