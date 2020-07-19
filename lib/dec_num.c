/* SPDX-License-Identifier: ISC */
/*
 * dec_num.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"

const char *gcfg_dec_num(gcfg_file_t *f, const char *str,
			 uint64_t *out, uint64_t max)
{
	const char *start = str;
	uint64_t x;

	if (*str < '0' || *str > '9')
		goto fail_num;

	if (str[0] == '0' && (str[1] >= '0' && str[1] <= '9'))
		goto fail_lead0;

	*out = 0;

	while (*str >= '0' && *str <= '9') {
		x = ((uint64_t)*(str++) & 0x00FF) - '0';

		if ((*out) > (max / 10) || ((*out) * 10) > (max - x))
			goto fail_ov;

		*out = (*out) * 10 + x;
	}

	return str;
fail_lead0:
	if (f != NULL)
		f->report_error(f, "numbers must not use leading zeros");
	return NULL;
fail_num:
	if (f != NULL)
		f->report_error(f, "expected digit instead of '%.6s...", str);
	return NULL;
fail_ov:
	if (f != NULL)
		f->report_error(f, "numeric overflow in '%.6s...'", start);
	return NULL;
}

#ifdef BUILD_TEST
typedef struct {
	const char *input;
	uint64_t maximum;
	uint64_t result;
} test_data_t;

static test_data_t must_work[] = {
	{ "0", 10, 0 },
	{ "7", 10, 7 },
	{ "65535", (1UL << 32UL) - 1, 0xFFFF },
	{ "65535asdf", (1UL << 32UL) - 1, 0xFFFF },
	{ "18446744073709551615", 0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL },
};

static test_data_t must_not_work[] = {
	{ "", 10, 0 },
	{ "a", 10, 0 },
	{ "00", 10, 0 },
	{ "07", 10, 0 },
	{ "12", 10, 0 },
	{ "32768", 32767, 0 },
	{ "18446744073709551616", 0xFFFFFFFFFFFFFFFFUL, 0 },
};

int main(void)
{
	gcfg_file_t df;
	uint64_t out;
	size_t i;

	for (i = 0; i < sizeof(must_work) / sizeof(must_work[0]); ++i) {
		dummy_file_init(&df, must_work[i].input);

		if (gcfg_dec_num(&df, must_work[i].input, &out,
				 must_work[i].maximum) == NULL) {
			fprintf(stderr, "'%s' was rejected!\n",
				must_work[i].input);
			return EXIT_FAILURE;
		}

		if (out != must_work[i].result) {
			fprintf(stderr, "'%s' was parsed as '%lu'!\n",
				must_work[i].input, (unsigned long)out);
			return EXIT_FAILURE;
		}

		dummy_file_cleanup(&df);
	}

	for (i = 0; i < sizeof(must_not_work) / sizeof(must_not_work[0]); ++i) {
		dummy_file_init(&df, must_not_work[i].input);

		if (gcfg_dec_num(&df, must_not_work[i].input, &out,
				 must_not_work[i].maximum) != NULL) {
			fprintf(stderr, "'%s' was accepted!\n",
				must_work[i].input);
			return EXIT_FAILURE;
		}

		dummy_file_cleanup(&df);
	}

	return EXIT_SUCCESS;
}
#endif
