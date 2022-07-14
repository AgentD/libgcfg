/* SPDX-License-Identifier: ISC */
/*
 * parse_enum.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

#include <string.h>

const char *gcfg_parse_enum(gcfg_file_t *f, const char *in,
			    const gcfg_enum_t *tokens, gcfg_value_t *out)
{
	size_t i, len;

	for (i = 0; tokens[i].name != NULL; ++i) {
		len = strlen(tokens[i].name);

		if (strncmp(tokens[i].name, in, len) != 0)
			continue;

		if (in[len] == ' ' || in[len] == '\t' || in[len] == '\0')
			break;
	}

	if (tokens[i].name == NULL) {
		if (f != NULL)
			f->report_error(f, "unexpected '%.5s...'", in);
		return NULL;
	}

	out->type = GCFG_VALUE_ENUM;
	out->flags = 0;
	out->cidr_mask = 0;
	out->data.enum_value = tokens[i].value;
	return in + len;
}
