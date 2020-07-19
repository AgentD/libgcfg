/* SPDX-License-Identifier: ISC */
/*
 * parse_boolean.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "gcfg.h"
#include "util.h"

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

const char *gcfg_parse_boolean(gcfg_file_t *f, const char *in, int *out)
{
	size_t i, len;

	for (i = 0; i < sizeof(mapping) / sizeof(mapping[0]); ++i) {
		len = mapping[i].len;

		if (strncmp(mapping[i].str, in, len) != 0)
			continue;

		if (in[len] != ' ' && in[len] != '\t' && in[len] != '\0')
			continue;

		*out = (int)mapping[i].value;
		return in + len;
	}

	f->report_error(f, "expected boolean value, found '%.6s...'", in);
	return NULL;
}

#ifdef BUILD_TEST
static const struct {
	const char *in;
	int out;
	int ret;
} testvec[] = {
	{ "true", 1, 0 },
	{ "yes", 1, 0 },
	{ "on", 1, 0 },
	{ "false", 0, 0 },
	{ "off", 0, 0 },
	{ "no", 0, 0 },
	{ "true ", 1, 0 },
	{ "yes ", 1, 0 },
	{ "on ", 1, 0 },
	{ "false ", 0, 0 },
	{ "off ", 0, 0 },
	{ "no ", 0, 0 },
	{ "trueA", 0, -1 },
	{ "yes0", 0, -1 },
	{ "on]", 0, -1 },
	{ "falseS", 0, -1 },
	{ "offD", 0, -1 },
	{ "noF", 0, -1 },
	{ "banana", 0, -1 },
	{ "", 0, -1 },
};

static void test_case(gcfg_file_t *df, size_t i)
{
	const char *ret;
	int out;

	ret = gcfg_parse_boolean(df, testvec[i].in, &out);

	if ((ret != NULL && testvec[i].ret != 0) ||
	    (ret == NULL && testvec[i].ret == 0)) {
		fprintf(stderr, "Wrong return status for %zu\n", i);
		fprintf(stderr, "Input: '%s' was %s\n",
			testvec[i].in,
			ret == NULL ? "not accepted" : "accepted");
		exit(EXIT_FAILURE);
	}

	if (ret == NULL)
		return;

	if (out != testvec[i].out) {
		fprintf(stderr, "Mismatch for %zu\n", i);
		fprintf(stderr, "'%s' was parsed as '%d'\n",
			testvec[i].in, out);
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
