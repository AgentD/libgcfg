/* SPDX-License-Identifier: ISC */
/*
 * parse_ipv6.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

#include <string.h>

static const char *h16(gcfg_file_t *f, const char *str, uint16_t *out)
{
	int i;

	if (gcfg_xdigit(*str) < 0)
		goto fail_num;

	*out = 0;

	for (i = 0; i < 4 && gcfg_xdigit(str[i]) >= 0; ++i)
		*out = (uint16_t)(*out << 4) | (uint16_t)gcfg_xdigit(str[i]);

	return str + i;
fail_num:
	if (f != NULL) {
		f->report_error(f, "expected hex digit instead of '%.6s...",
				str);
	}
	return NULL;
}

static const char *ls32(const char *str, uint16_t *hi, uint16_t *lo)
{
	const char *temp;
	uint32_t ipv4;

	temp = gcfg_ipv4address(NULL, str, &ipv4);
	if (temp != NULL) {
		*hi = (uint16_t)((ipv4 >> 16) & 0x0000FFFF);
		*lo = (uint16_t)( ipv4        & 0x0000FFFF);
		return temp;
	}

	str = h16(NULL, str, hi);
	if (str == NULL)
		return NULL;

	return *str == ':' ? h16(NULL, str + 1, lo) : NULL;
}

const char *gcfg_parse_ipv6(gcfg_file_t *f, const char *str,
			    gcfg_net_addr_t *ret)
{
	uint16_t v[8], w[8];
	const char *temp;
	uint64_t mask;
	int i, j;

	memset(v, 0, sizeof(v));

	for (i = 0; i < 8; ++i) {
		if (str[0] == ':' && str[1] == ':') {
			str += 2;
			break;
		}

		if (i > 0) {
			if (*str != ':')
				goto fail_colon;
			++str;
		}

		if (i == 6) {
			temp = ls32(str, v + i, v + i + 1);
			if (temp != NULL) {
				str = temp;
				i += 2;
				break;
			}
		}

		str = h16(f, str, v + i);
		if (str == NULL)
			return NULL;
	}

	if (i < 8) {
		for (j = 0; j < 8; ++j) {
			if (*str == ' ' || *str == '\t' ||
			    *str == '/' || *str == '\0') {
				break;
			}
			if (j > 0) {
				if (*str != ':')
					goto fail_colon;
				++str;
			}

			temp = ls32(str, w + j, w + j + 1);
			if (temp != NULL &&
			    (*temp == ' ' || *temp == '\t' ||
			     *temp == '/' || *temp == '\0')) {
				j += 2;
				str = temp;
				break;
			}

			str = h16(f, str, w + j);
			if (str == NULL)
				return NULL;
		}

		if ((j + i) > 7) {
			f->report_error(f, "overlong IPv6 address");
			return NULL;
		}

		if (j > 0)
			memcpy(v + (8 - j), w, (size_t)j * sizeof(uint16_t));
	}

	ret->flags = GCFG_NET_ADDR_IPV6;

	if (*str == '/') {
		str = gcfg_dec_num(f, str + 1, &mask, 128);
		if (str == NULL)
			return NULL;

		ret->cidr_mask = (uint32_t)(mask & 0x00FF);
		ret->flags |= GCFG_NET_ADDR_HAVE_MASK;
	} else {
		ret->cidr_mask = 128;
	}

	memcpy(ret->raw.ipv6, v, sizeof(v));
	return str;
fail_colon:
	f->report_error(f, "expected ':', found '%c'", *str);
	return NULL;
}
