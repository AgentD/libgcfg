/* SPDX-License-Identifier: ISC */
/*
 * parse_ipv4.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

const char *gcfg_parse_ipv4(gcfg_file_t *f, const char *in,
			    gcfg_value_t *ret)
{
	uint64_t temp;

	in = gcfg_ipv4address(f, in, &ret->data.ipv4);
	if (in == NULL)
		return NULL;

	ret->type = GCFG_VALUE_IPV4;
	ret->flags = 0;

	if (*in == ' ' || *in == '\t' || *in == '\0') {
		ret->cidr_mask = 32;
	} else {
		if (*in != '/') {
			f->report_error(f, "unexpected '%c' after IP address",
					*in);
			return NULL;
		}

		in = gcfg_dec_num(f, in + 1, &temp, 32);
		if (in == NULL)
			return NULL;

		ret->cidr_mask = (uint8_t)(temp & 0x00FF);
		ret->flags |= GCFG_NET_ADDR_HAVE_MASK;
	}

	return in;
}
