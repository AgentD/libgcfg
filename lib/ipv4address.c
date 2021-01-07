/* SPDX-License-Identifier: ISC */
/*
 * ipv4address.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

const char *gcfg_ipv4address(gcfg_file_t *f, const char *str, uint32_t *out)
{
	const char *start = str;
	uint64_t temp;
	int i;

	*out = 0;

	for (i = 0; i < 4; ++i) {
		if (i > 0 && *(str++) != '.')
			goto fail;

		str = gcfg_dec_num(f, str, &temp, 255);
		if (str == NULL)
			return NULL;

		*out = ((*out) << 8) | ((uint32_t)(temp & 0x00FF));
	}
	return str;
fail:
	if (f != NULL)
		f->report_error(f, "malformed IPv4 address '%.5s...'", start);
	return NULL;
}
