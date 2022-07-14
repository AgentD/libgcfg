/* SPDX-License-Identifier: ISC */
/*
 * parse_mac_addr.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

const char *gcfg_parse_mac_addr(gcfg_file_t *f, const char *in,
				gcfg_value_t *ret)
{
	const char *start = in;
	uint64_t val = 0;
	int i;

	for (i = 0; i < 6; ++i) {
		if (i > 0 && *(in++) != ':')
			goto fail;

		if (gcfg_xdigit(in[0]) < 0 || gcfg_xdigit(in[1]) < 0)
			goto fail;

		val <<= 8;
		val |= (uint64_t)(gcfg_xdigit(in[0]) << 4);
		val |= (uint64_t)gcfg_xdigit(in[1]);
		in += 2;
	}

	ret->data.mac.vendor = (val >> 24) & 0x00FFFFFF;
	ret->data.mac.device =  val        & 0x00FFFFFF;
	ret->cidr_mask       = 0;
	ret->type            = GCFG_VALUE_MAC;
	ret->flags           = 0;
	return in;
fail:
	f->report_error(f, "invalid MAC address '%.*s...'",
			(int)(in - start + 1), start);
	return NULL;
}
