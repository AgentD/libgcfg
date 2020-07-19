/* SPDX-License-Identifier: ISC */
/*
 * is_valid_utf8.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const uint32_t min_val_by_count[] = {
	0xFFFFFFFF,
	0x00000020,
	0x00000080,
	0x00000800,
	0x00010000,
	0x00200000,
	0x04000000,
};

static const uint8_t mask_by_count[] = {
	0xFF, 0xFF, 0x1F, 0x0F, 0x07, 0x03
};

static const uint8_t seq_length[64] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 5, 0,
};

bool gcfg_is_valid_cp(uint32_t cp)
{
	if (cp > 0x10FFFF)
		return false;

	/* surrogate pairs */
	if ((cp >= 0xD800 && cp <= 0xDBFF) || (cp >= 0xDC00 && cp <= 0xDFFF))
		return false;

	/* stable set of non-characters */
	if (cp >= 0xFDD0 && cp <= 0xFDEF)
		return false;

	if ((cp & 0x0000FFFE) == 0xFFFE)
		return false;

	return true;
}

bool gcfg_is_valid_utf8(const uint8_t *str, size_t len)
{
	size_t i, count = 0;
	uint32_t cp;
	uint8_t x;

	while (len--) {
		x = *(str++);
		count = seq_length[(x >> 2) & 0x3F];
		cp = x & mask_by_count[count];

		if (count > 1) {
			if (len < (count - 1))
				return false;

			for (i = 1; i < count; ++i) {
				x = *(str++);
				--len;

				if ((x & 0xC0) != 0x80)
					return false;

				cp = (cp << 6) | (x & 0x3F);
			}
		}

		if (cp < min_val_by_count[count] && cp != 0x09)
			return false;

		if (!gcfg_is_valid_cp(cp) || cp == 0x7F)
			return false;
	}

	return true;
}

#ifdef BUILD_TEST
static const char *must_work[] = {
	"Hello, world!",
	"这是一些中文。",
	"هذه هي بعض النصوص العربي",
	"019\t\t { foo } äöü ;",
};

static const char *must_not_work[] = {
	"Foo\nBar",
	"Foo\rBar",
	"Foo\bBar",
	"\xC0\xA0",
	"\x7F",
	"\xFF",
};

int main(void)
{
	size_t i;

	for (i = 0; i < sizeof(must_work) / sizeof(must_work[0]); ++i) {
		if (!gcfg_is_valid_utf8((const uint8_t *)must_work[i],
					strlen(must_work[i]))) {
			fprintf(stderr, "'%s' was rejected!\n", must_work[i]);
			return EXIT_FAILURE;
		}
	}

	for (i = 0; i < sizeof(must_not_work) / sizeof(must_not_work[0]); ++i) {
		if (gcfg_is_valid_utf8((const uint8_t *)must_not_work[i],
				       strlen(must_not_work[i]))) {
			fprintf(stderr, "entry %lu was accepted!\n",
				(unsigned long)i);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
#endif
