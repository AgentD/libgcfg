
/* SPDX-License-Identifier: ISC */
/*
 * parse_number.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

const char *gcfg_parse_number(gcfg_file_t *f, const char *in,
			      gcfg_value_t *out, size_t index)
{
	bool negative = false, percentage = false;
	int32_t exponent;
	uint64_t temp;
	int64_t value;
	int i;

	if (index >= 4)
		goto fail_index;

	/* parse value */
	if (*in == '-')
		negative = true;

	if (*in == '+' || *in == '-')
		++in;

	in = gcfg_dec_num(f, in, &temp, 0x7FFFFFFFFFFFFFFF);
	if (in == NULL)
		return NULL;

	value = (int64_t)temp;
	exponent = 0;

	if (*in == '.') {
		++in;
		for (i = 0; in[i] >= '0' && in[i] <= '9'; ++i) {
			if (value >= (0x7FFFFFFFFFFFFFFFL / 10))
				goto fail_fract;
			if (exponent == INT32_MIN)
				goto fail_fract;

			value *= 10;
			exponent--;
		}

		in = gcfg_dec_num(f, in, &temp, 0x7FFFFFFFFFFFFFFF);
		if (in == NULL)
			return NULL;

		value += (int64_t)temp;
	}

	/* parse exponent */
	switch (*in) {
	case 'e':
	case 'E':
		i = (in[1] == '-');
		in += (in[1] == '-' || in[1] == '+') ? 2 : 1;

		in = gcfg_dec_num(f, in, &temp, 0x7FFFFFFF);
		if (in == NULL)
			return NULL;

		if (i) {
			if (exponent < (INT32_MIN + (int32_t)temp))
				goto fail_exp;
			exponent -= (int32_t)temp;
		} else {
			if (exponent > (INT32_MAX - (int32_t)temp))
				goto fail_exp;
			exponent += (int32_t)temp;
		}
		break;
	case '%':
		if (exponent < (INT32_MIN + 2))
			goto fail_exp;
		++in;
		exponent -= 2;
		percentage = true;
		break;
	default:
		break;
	}

	/* store result */
	out->flags = 0;
	out->cidr_mask = 0;
	out->type = percentage ? GCFG_VALUE_PERCENTAGE : GCFG_VALUE_NUMBER;
	out->data.number[index].value = negative ? -value : value;
	out->data.number[index].exponent = exponent;
	return in;
fail_index:
	if (f != NULL) {
		f->report_error(f, "[BUG] vector index out of bounds");
	}
	return NULL;
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
