/* SPDX-License-Identifier: ISC */
/*
 * parse_boolean.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

#include <string.h>

static const struct {
	const char *str;
	size_t len;
	size_t value;
} mapping[] = {
	{ "on", 2, 1 },
	{ "yes", 3, 1 },
	{ "true", 4, 1 },
	{ "off", 3, 0 },
	{ "no", 2, 0 },
	{ "false", 5, 0 },
};

const char *gcfg_parse_boolean(gcfg_file_t *f, const char *in,
			       gcfg_value_t *out)
{
	size_t i, len;

	for (i = 0; i < sizeof(mapping) / sizeof(mapping[0]); ++i) {
		len = mapping[i].len;

		if (strncmp(mapping[i].str, in, len) != 0)
			continue;

		if (in[len] != ' ' && in[len] != '\t' && in[len] != '\0')
			continue;

		out->type = GCFG_VALUE_BOOLEAN;
		out->flags = 0;
		out->cidr_mask = 0;
		out->data.boolean = (mapping[i].value != 0);
		return in + len;
	}

	f->report_error(f, "expected boolean value, found '%.6s...'", in);
	return NULL;
}
