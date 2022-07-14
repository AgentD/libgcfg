/* SPDX-License-Identifier: ISC */
/*
 * parse_size.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

const char *gcfg_parse_size(gcfg_file_t *f, const char *in,
			    gcfg_value_t *ret)
{
	const char *start = in;
	uint64_t value, shift;

	in = gcfg_dec_num(f, in, &value, 0xFFFFFFFFFFFFFFFF);
	if (in == NULL)
		return NULL;

	switch (*in) {
	case 'k':
	case 'K':
		++in;
		shift = 10;
		break;
	case 'm':
	case 'M':
		++in;
		shift = 20;
		break;
	case 'g':
	case 'G':
		++in;
		shift = 30;
		break;
	case 't':
	case 'T':
		++in;
		shift = 40;
		break;
	default:
		shift = 0;
		break;
	}

	if (value > (0xFFFFFFFFFFFFFFFFUL >> shift)) {
		f->report_error(f, "numeric overflow in %.6s...", start);
		return NULL;
	}

	ret->data.size = value << shift;
	ret->flags = 0;
	ret->cidr_mask = 0;
	ret->type = GCFG_VALUE_SIZE;
	return in;
}
