/* SPDX-License-Identifier: ISC */
/*
 * parse_vector.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

const char *gcfg_parse_vector(gcfg_file_t *f, const char *in,
			      gcfg_value_t *out, size_t count)
{
	size_t i;

	if (*in != '(') {
		f->report_error(f, "expected '(', found '%c'", *in);
		return NULL;
	}

	++in;

	for (i = 0; i < count; ++i) {
		if (i > 0) {
			if (*in != ',') {
				f->report_error(f, "expected ',', found '%c'",
						*in);
				return NULL;
			}
			++in;
		}

		while (*in == ' ' || *in == '\t')
			++in;

		in = gcfg_parse_number(f, in, out, i);
		if (in == NULL)
			return NULL;

		while (*in == ' ' || *in == '\t')
			++in;
	}

	if (*in != ')') {
		f->report_error(f, "expected ')', found '%c'", *in);
		return NULL;
	}

	++in;
	while (*in == ' ' || *in == '\t')
		++in;

	switch (count) {
	case 2:  out->type = GCFG_VALUE_VEC2; break;
	case 3:  out->type = GCFG_VALUE_VEC3; break;
	case 4:  out->type = GCFG_VALUE_VEC4; break;
	default: out->type = GCFG_VALUE_NUMBER; break;
	}

	return in;
}
