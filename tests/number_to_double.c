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
	gcfg_value_t input;
	double result;
} test_vec[] = {
	{ { .data = { .number = {{1337}, {0}} } }, 1337.0 },
	{ { .data = { .number = {{-1337}, {0}} } }, -1337.0 },
	{ { .data = { .number = {{1337}, {-2}} } }, 13.37 },
	{ { .data = { .number = {{-1337}, {-2}} } }, -13.37 },
	{ { .data = { .number = {{42}, {2}} } }, 4200.0 },
	{ { .data = { .number = {{-42}, {2}} } }, -4200.0 },
};

int main(void)
{
	double diff, expected, ret;
	size_t i;

	for (i = 0; i < sizeof(test_vec) / sizeof(test_vec[0]); ++i) {
		expected = test_vec[i].result;

		ret = gcfg_number_to_double(&test_vec[i].input, 0);

		diff = fabs(expected - ret);

		if (diff > 1e-10) {
			fprintf(stderr, "'%lde%d' was converted "
				"to %f instead of %f (diff %f)!\n",
				(long)test_vec[i].input.data.number.value[0],
				test_vec[i].input.data.number.exponent[0],
				ret, expected, diff);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
