/* SPDX-License-Identifier: ISC */
/*
 * parse_string.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "gcfg.h"
#include "util.h"

static int encode_utf8(uint8_t *out, uint32_t cp)
{
	int i = 1;

	if (cp >= 0x00010000) {
		*(out++) = (uint8_t)(0xF0 | ((cp >> 18) & 0x07));
		goto out3;
	}

	if (cp >= 0x00000800) {
		*(out++) = (uint8_t)(0xE0 | ((cp >> 12) & 0x0F));
		goto out2;
	}

	if (cp >= 0x00000080) {
		*(out++) = (uint8_t)(0xC0 | ((cp >> 6) & 0x1F));
		goto out1;
	}

	*out = (uint8_t)cp;
	return i;
out3:	*(out++) = (uint8_t)(0x80 | ((cp >> 12) & 0x3F)); ++i;
out2:	*(out++) = (uint8_t)(0x80 | ((cp >>  6) & 0x3F)); ++i;
out1:	*(out++) = (uint8_t)(0x80 | ( cp        & 0x3F)); ++i;
	return i;
}

const char *gcfg_parse_string(gcfg_file_t *f, const char *in, char *out)
{
	char *dst = out;
	uint32_t cp;
	size_t i;

	if (*(in++) != '"')
		goto fail_start;

	while (*in != '"') {
		if (*in == '\0')
			goto fail_eol;

		if (*in == '\\') {
			++in;
			switch (*(in++)) {
			case 'b': *(dst++) = '\b'; break;
			case 't': *(dst++) = '\t'; break;
			case 'n': *(dst++) = '\n'; break;
			case 'r': *(dst++) = '\r'; break;
			case '\\': *(dst++) = '\\'; break;
			case '"': *(dst++) = '"'; break;
			case 'u':
			case 'U':
				if (*(in++) != '+')
					goto fail_esc;

				cp = 0;
				i = 0;

				while (i < 6 && gcfg_xdigit(*in) >= 0) {
					cp <<= (uint32_t)4;
					cp |= (uint32_t)gcfg_xdigit(*(in++));
				}

				if (!gcfg_is_valid_cp(cp))
					goto fail_cp;

				/* XXX: assumes 8 bit chars */
				dst += encode_utf8((uint8_t *)dst, cp);
				break;
			default:
				goto fail_esc;
			}
			continue;
		}

		*(dst++) = *(in++);
	}

	*dst = '\0';
	return in + 1;
fail_start:
	if (f != NULL)
		f->report_error(f, "expected string starting with '\"'");
	return NULL;
fail_esc:
	if (f != NULL)
		f->report_error(f, "unknown escape sequence in string");
	return NULL;
fail_eol:
	if (f != NULL)
		f->report_error(f, "expected '\"' before end of line");
	return NULL;
fail_cp:
	if (f != NULL)
		f->report_error(f, "invalid unicode code point in string");
	return NULL;
}

#ifdef BUILD_TEST
static const struct {
	const char *in;
	const char *out;
	int ret;
} testvec[] = {
	{ "", NULL, -1 },
	{ "\"", NULL, -1 },
	{ "\"\"", "", 0 },
	{ "\"foo\"", "foo", 0 },
	{ "\"\\\\\"", "\\", 0 },
	{ "\"\\\"\"", "\"", 0 },
	{ "\"b\\bt\\tn\\nr\\r\"", "b\bt\tn\nr\r", 0 },
	{ "\"\\a\"", NULL, -1 },
	{ "\"你好！\"", "你好！", 0 },
	{ "\"\\U+0041\"", "A", 0 },
	{ "\"\\U+00A5\"", "\xC2\xA5", 0 },
	{ "\"\\U+FFFF\"", NULL, -1 },
	{ "\"\\U+FFFE\"", NULL, -1 },
	{ "\"\\U+1FFFF\"", NULL, -1 },
	{ "\"\\U+1FFFE\"", NULL, -1 },
	{ "\"\\U+10FFFE\"", NULL, -1 },
	{ "\"\\U+10FFFF\"", NULL, -1 },
	{ "\"\\U+110000\"", NULL, -1 },
};

static void test_case(gcfg_file_t *df, size_t i)
{
	char buffer[128];
	const char *ret;

	ret = gcfg_parse_string(df, testvec[i].in, buffer);

	if ((ret == NULL && testvec[i].ret == 0) ||
	    (ret != NULL && testvec[i].ret != 0)) {
		fprintf(stderr, "Wrong return status for %zu\n", i);
		fprintf(stderr, "Input: '%s' was %s\n",
			testvec[i].in,
			ret == NULL ? "not accepted" : "accepted");
		exit(EXIT_FAILURE);
	}

	if (ret != NULL && strcmp(buffer, testvec[i].out) != 0) {
		fprintf(stderr, "Mismatch for %zu\n", i);
		fprintf(stderr, "Expected: %s\n", testvec[i].out);
		fprintf(stderr, "Received: %s\n", buffer);
		exit(EXIT_FAILURE);
	}
}

int main(void)
{
	gcfg_file_t df;
	size_t i;

	for (i = 0; i < sizeof(testvec) / sizeof(testvec[0]); ++i) {
		dummy_file_init(&df, testvec[i].in);
		test_case(&df, i);
		dummy_file_cleanup(&df);
	}

	return EXIT_SUCCESS;
}
#endif
