/* SPDX-License-Identifier: ISC */
/*
 * parse_uri.c
 *
 * Copyright (C) 2022 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

#include <string.h>

static bool is_unreserved(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9') || c == '-' || c == '.' ||
		c == '_' || c == '~';
}

static bool is_gen_delim(int c)
{
	return (c == ':') || (c == '/') || (c == '?') || (c == '#') ||
		(c == '[') || (c == ']') || (c == '@');
}

static bool is_sub_delim(int c)
{
	return (c == '!') || (c == '$') || (c == '&') || (c == '\'') ||
		(c == '(') || (c == ')') || (c == '*') || (c == '+') ||
		(c == ',') || (c == ';') || (c == '=');
}

static bool is_pchar(int c)
{
	return is_unreserved(c) || c == '%' || is_sub_delim(c) || c == '@';
}

static bool is_uri_char(int c)
{
	return c == '%' || is_unreserved(c) ||
		is_gen_delim(c) || is_sub_delim(c);
}

static bool have_char_in_uri(const char *ptr, int c)
{
	while (is_uri_char(*ptr)) {
		if (*ptr == c)
			return true;
		++ptr;
	}

	return false;
}

/*****************************************************************************/

static const char *scheme(gcfg_file_t *f, const char *in,
			  char *out, gcfg_value_t *uri)
{
	uri->data.uri.scheme = out;

	for (;;) {
		int c = *(in++);

		if (c >= 'A' && c <= 'Z')
			c = (c - 'A') + 'a';

		if (c >= 'a' && c <= 'z') {
			*(out++) = (char)c;
			continue;
		}

		if ((c >= '0' && c <= '9') || c == '+' ||
		    c == '-' || c == '.') {
			if (uri->data.uri.scheme == out)
				goto fail;
			*(out++) = (char)c;
			continue;
		}

		if (c == ':') {
			if (uri->data.uri.scheme == out)
				goto fail;
			*(out++) = '\0';
			break;
		} else {
			goto fail;
		}
	}

	return in;
fail:
	if (f != NULL)
		f->report_error(f, "expected URI starting with `<scheme>:`");
	return NULL;
}

static const char *userinfo(gcfg_file_t *f, const char *in,
			    char *out, gcfg_value_t *uri)
{
	uri->data.uri.userinfo = out;

	for (;;) {
		int c = *(in++);

		if (is_sub_delim(c) || is_unreserved(c) ||
		    c == ':' || c == '%') {
			*(out++) = (char)c;
			continue;
		}

		if (c == '@') {
			*(out++) = '\0';
			break;
		} else {
			goto fail;
		}
	}

	return in;
fail:
	if (f != NULL)
		f->report_error(f, "unexpected character in URI user info");
	return NULL;
}

static const char *hostname(gcfg_file_t *f, const char *in,
			    char *out, gcfg_value_t *uri)
{
	uint64_t port = 0;
	const char *end;

	uri->data.uri.host = out;

	if (*in == '[') {
		gcfg_value_t temp;

		++in;

		for (;;) {
			int c = *(in++);

			if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') ||
			    (c >= '0' && c <= '9') || c == ':' || c == '.') {
				*(out++) = (char)c;
				continue;
			}

			if (c == ']') {
				*out = '\0';
				break;
			} else {
				goto fail_addr_v6;
			}
		}

		if (gcfg_parse_ipv6(NULL, uri->data.uri.host, &temp) == NULL)
			goto fail_addr_v6;

		uri->flags |= GCFG_URI_HOST_IPV6;
	} else {
		uint32_t temp;

		end = gcfg_ipv4address(NULL, in, &temp);

		if (end == NULL) {
			for (;;) {
				if (is_unreserved(*in) ||
				    is_sub_delim(*in) || *in == '%') {
					*(out++) = *(in++);
				} else {
					if (uri->data.uri.host == out)
						goto fail_host_empty;
					break;
				}
			}

			uri->flags |= GCFG_URI_HOST_NAME;
		} else {
			while (in < end)
				*(out++) = *(in++);

			uri->flags |= GCFG_URI_HOST_IPV4;
		}
	}

	if (*in == ':') {
		end = gcfg_dec_num(NULL, in + 1, &port, 0x0FFFF);

		if (end == NULL) {
			if (f != NULL)
				f->report_error(f, "invalid port "
						"number in URI");
			return NULL;
		}

		uri->flags |= GCFG_URI_HAS_PORT;
		uri->data.uri.port = (uint16_t)(port & 0x0FFFF);
		in = end;
	}

	*out = '\0';
	return in;
fail_addr_v6:
	if (f != NULL)
		f->report_error(f, "malformed IPv6 literal in URI");
	return NULL;
fail_host_empty:
	if (f != NULL)
		f->report_error(f, "URI has an empty host name");
	return NULL;
}

static const char *path_rootless(const char *in, char *out)
{
	while (is_pchar(*in))
		*(out++) = *(in++);

	while (*in == '/') {
		while (*in == '/')
			++in;

		if (!is_pchar(*in))
			break;

		*(out++) = '/';
		while (is_pchar(*in))
			*(out++) = *(in++);
	}

	*out = '\0';
	return in;
}

/*****************************************************************************/

const char *gcfg_parse_uri(gcfg_file_t *f, const char *in,
			   char *out, gcfg_value_t *uri)
{
	memset(uri, 0, sizeof(*uri));

	uri->type = GCFG_VALUE_URI;

	/* <scheme> ':' */
	in = scheme(f, in, out, uri);
	if (in == NULL)
		return NULL;
	out += (strlen(out) + 1);

	if (in[0] == '/' && in[1] == '/') {
		in += 2;

		/* [<userinfo> '@'] */
		if (have_char_in_uri(in, '@')) {
			in = userinfo(f, in, out, uri);
			if (in == NULL)
				return NULL;
			out += (strlen(out) + 1);
		}

		/* <host> [':' <port>] */
		in = hostname(f, in, out, uri);
		if (in == NULL)
			return NULL;
		out += (strlen(out) + 1);

		/* <path_abempty> */
		uri->data.uri.path = out;

		while (*in == '/') {
			while (*in == '/')
				++in;

			if (!is_pchar(*in))
				break;

			*(out++) = '/';
			while (is_pchar(*in))
				*(out++) = *(in++);
		}

		if (out == uri->data.uri.path)
			*(out++) = '/';
		*(out++) = '\0';
	} else if (*in == '/') {
		if (!is_pchar(in[1]))
			goto fail_absolute;

		uri->data.uri.path = out;
		*(out++) = '/';
		++in;

		in = path_rootless(in, out);
		out += (strlen(out) + 1);
	} else if (is_pchar(*in)) {
		uri->data.uri.path = out;
		in = path_rootless(in, out);
	}

	/* ['?' (pchar | '/' | '?')*] */
	if (*in == '?') {
		++in;

		uri->data.uri.query = out;
		while (is_pchar(*in) || *in == '/' || *in == '?')
			*(out++) = *(in++);

		*(out++) = '\0';
	}

	/* ['#' (pchar | '/' | '?')*] */
	if (*in == '#') {
		++in;

		uri->data.uri.fragment = out;
		while (is_pchar(*in) || *in == '/' || *in == '?')
			*(out++) = *(in++);

		*(out++) = '\0';
	}

	return in;
fail_absolute:
	if (f != NULL) {
		f->report_error(f, "expected absolute path in "
				"URI after `<scheme>:/`");
	}
	return NULL;
}
