/* SPDX-License-Identifier: ISC */
/*
 * package.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

#include "gcfg.h"

#define STR(x) #x
#define STRVALUE(x) STR(x)

#define CFG_PATH STRVALUE(CFGPATH)

typedef struct {
	char *name;
	char *maintainer;
	char *checksum;
	char *desc;
	char *url;
} package_t;

static void pkg_cleanup(package_t *pkg)
{
	free(pkg->name);
	free(pkg->maintainer);
	free(pkg->checksum);
	free(pkg->desc);
	free(pkg->url);
	free(pkg);
}

static void *copy_string_field(gcfg_file_t *file, char **field,
			       const gcfg_value_t *value)
{
	(void)file;
	assert(*field == NULL);
	*field = strdup(value->data.string);
	assert(*field != NULL);
	return *field;
}

static void *pkg_set_name(gcfg_file_t *file, void *obj,
			  const gcfg_value_t *value)
{
	package_t *pkg = obj;
	return copy_string_field(file, &pkg->name, value);
}

static void *pkg_set_maintainer(gcfg_file_t *file, void *obj,
				const gcfg_value_t *value)
{
	package_t *pkg = obj;
	return copy_string_field(file, &pkg->maintainer, value);
}

static void *pkg_set_checksum(gcfg_file_t *file, void *obj,
			     const gcfg_value_t *value)
{
	package_t *pkg = obj;
	return copy_string_field(file, &pkg->checksum, value);
}

static void *pkg_set_desc(gcfg_file_t *file, void *obj,
			  const gcfg_value_t *value)
{
	package_t *pkg = obj;
	return copy_string_field(file, &pkg->desc, value);
}

static void *pkg_set_url(gcfg_file_t *file, void *obj,
			  const gcfg_value_t *value)
{
	package_t *pkg = obj;
	(void)file;
	assert(pkg->url == NULL);
	asprintf(&pkg->url, "%s from %s using %s",
		 value->data.uri.path, value->data.uri.host,
		 value->data.uri.scheme);
	assert(pkg->url != NULL);
	return pkg;
}

GCFG_BEGIN_KEYWORDS(kw_package)
	GCFG_KEYWORD_STRING("name", NULL, pkg_set_name, NULL),
	GCFG_KEYWORD_STRING("maintainer", NULL, pkg_set_maintainer, NULL),
	GCFG_KEYWORD_STRING("checksum", NULL, pkg_set_checksum, NULL),
	GCFG_KEYWORD_STRING("description", NULL, pkg_set_desc, NULL),
	GCFG_KEYWORD_URI("url", NULL, pkg_set_url, NULL),
GCFG_END_KEYWORDS();

/*****************************************************************************/

int main(void)
{
	gcfg_file_t *file = gcfg_file_open(CFG_PATH);
	package_t *pkg;

	if (file == NULL)
		return EXIT_FAILURE;

	pkg = calloc(1, sizeof(*pkg));
	if (pkg == NULL) {
		file->report_error(file, "out of memory");
		gcfg_file_close(file);
		return EXIT_FAILURE;
	}

	if (gcfg_parse_file(file, kw_package, pkg)) {
		gcfg_file_close(file);
		pkg_cleanup(pkg);
		return EXIT_FAILURE;
	}

	gcfg_file_close(file);

	printf("Name: `%s`\n", pkg->name);
	printf("Description: `%s`\n", pkg->desc);
	printf("Maintainer: `%s`\n", pkg->maintainer);
	printf("Checksum: `%s`\n", pkg->checksum);
	printf("URL: `%s`\n", pkg->url);

	pkg_cleanup(pkg);
	return EXIT_SUCCESS;
}
