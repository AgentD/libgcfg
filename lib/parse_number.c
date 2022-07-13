
/* SPDX-License-Identifier: ISC */
/*
 * parse_number.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

const char *gcfg_parse_number(gcfg_file_t *f, const char *in,
			      gcfg_number_t *num)
{
	int32_t exponent;
	uint64_t temp;
	int i;

	num->flags = 0;
	num->exponent = 0;

	if (*in == '-')
		num->flags |= GCFG_NUM_NEGATIVE;

	if (*in == '+' || *in == '-')
		++in;

	in = gcfg_dec_num(f, in, &num->value, 0xFFFFFFFFFFFFFFFF);
	if (in == NULL)
		return NULL;

	if (*in == '.') {
		++in;
		for (i = 0; in[i] >= '0' && in[i] <= '9'; ++i) {
			if (num->value >= (0xFFFFFFFFFFFFFFFFUL / 10))
				goto fail_fract;
			if (num->exponent == INT32_MIN)
				goto fail_fract;

			num->value *= 10;
			num->exponent--;
		}

		in = gcfg_dec_num(f, in, &temp, 0xFFFFFFFFFFFFFFFF);
		if (in == NULL)
			return NULL;

		num->value += temp;
	}

	switch (*in) {
	case 'e':
	case 'E':
		i = (in[1] == '-');
		in += (in[1] == '-' || in[1] == '+') ? 2 : 1;

		in = gcfg_dec_num(f, in, &temp, 0x7FFFFFFF);
		if (in == NULL)
			return NULL;

		exponent = (int32_t)temp;
		if (i) {
			if (num->exponent < (INT32_MIN + exponent))
				goto fail_exp;
			num->exponent -= exponent;
		} else {
			if (num->exponent > (INT32_MAX - exponent))
				goto fail_exp;
			num->exponent += exponent;
		}
		break;
	case '%':
		if (num->exponent < (INT32_MIN + 2))
			goto fail_exp;
		++in;
		num->exponent -= 2;
		num->flags |= GCFG_NUM_PERCENTAGE;
		break;
	default:
		break;
	}

	return in;
fail_exp:
	if (f != NULL) {
		f->report_error(f, "numeric oveflow/underflow in exponent");
	}
	return NULL;
fail_fract:
	if (f != NULL) {
		f->report_error(f, "too many fraction digits, "
				"number would be trucated");
	}
	return NULL;
}
