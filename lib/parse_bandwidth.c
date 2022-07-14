/* SPDX-License-Identifier: ISC */
/*
 * parse_bandwidth.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

#define PREFIX(bin, dec) \
		++in; \
		if (*in == 'i' || *in == 'I') { \
			++in; scale = (bin); \
		} else { \
			scale = (dec); \
		}

const char *gcfg_parse_bandwidth(gcfg_file_t *f, const char *in,
				 gcfg_value_t *ret)
{
	const char *start = in;
	uint64_t value, scale;

	in = gcfg_dec_num(f, in, &value, 0xFFFFFFFFFFFFFFFF);
	if (in == NULL)
		return NULL;

	switch (*in) {
	case 'k':
	case 'K':
		PREFIX(1UL << 10UL, 1000UL)
		break;
	case 'm':
	case 'M':
		PREFIX(1UL << 20UL, 1000000UL)
		break;
	case 'g':
	case 'G':
		PREFIX(1UL << 30UL, 1000000000UL)
		break;
	case 't':
	case 'T':
		PREFIX(1UL << 40UL, 1000000000000UL);
		break;
	default:
		scale = 1;
		break;
	}

	if ((in[0] == 'b' || in[0] == 'B') && (in[1] == 'p' || in[1] == 'P') &&
	    (in[2] == 's' || in[2] == 'S')) {
		in += 3;
		scale *= 8;
	} else if ((in[0] == 'b' || in[0] == 'B') && (in[1] == 'i' || in[1] == 'I') &&
		   (in[2] == 't' || in[2] == 'T')) {
		in += 3;
	} else if (*in == 'B') {
		++in;
		scale *= 8;
	} else if (*in == 'b') {
		++in;
	}

	if (value > (0xFFFFFFFFFFFFFFFFUL / scale)) {
		f->report_error(f, "numeric overflow in %.6s...", start);
		return NULL;
	}

	ret->type = GCFG_VALUE_BANDWIDTH;
	ret->cidr_mask = 0;
	ret->flags = 0;
	ret->data.bandwidth = value * scale;
	return in;
}
