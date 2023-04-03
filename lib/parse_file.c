/* SPDX-License-Identifier: ISC */
/*
 * parse_file.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

#include <string.h>

static const char *skip_space(const char *ptr)
{
	while (*ptr == ' ' || *ptr == '\t')
		++ptr;
	return ptr;
}

static int is_line_end(int c)
{
	return c == '\0' || c == '#';
}

static const char *find_keyword(gcfg_file_t *file,
				const gcfg_keyword_t *keywords,
				const char *ptr,
				const gcfg_keyword_t **out)
{
	const gcfg_keyword_t *kwd;
	size_t len;

	if (!(*ptr >= 'a' && *ptr <= 'z') &&
	    !(*ptr >= 'A' && *ptr <= 'Z') &&
	    !(*ptr >= '0' && *ptr <= '9')) {
		file->report_error(file, "Expected keyword, found '%.6s...'",
				   ptr);
		return NULL;
	}

	for (kwd = keywords; kwd->name != NULL; ++kwd) {
		len = strlen(kwd->name);

		if (strncmp(ptr, kwd->name, len) != 0)
			continue;

		if (ptr[len] == ' ' || ptr[len] == '\t' ||
		    is_line_end(ptr[len])) {
			*out = kwd;
			return skip_space(ptr + len);
		}
	}

	file->report_error(file, "Unknown keyword '%.6s...'", ptr);
	return NULL;
}

static const char *apply_arg(gcfg_file_t *file, const gcfg_keyword_t *kwd,
			     const char *ptr, void *parent, void **child_out)
{
	gcfg_value_t val;
	char *strval;

	memset(&val, 0, sizeof(val));

	switch (kwd->arg) {
	case GCFG_VALUE_NONE:
		val.type = GCFG_VALUE_NONE;
		break;
	case GCFG_VALUE_BOOLEAN:
		ptr = gcfg_parse_boolean(file, ptr, &val);
		break;
	case GCFG_VALUE_STRING:
		strval = file->buffer;
		ptr = gcfg_parse_string(file, ptr, strval);
		val.type = GCFG_VALUE_STRING;
		val.data.string = strval;
		break;
	case GCFG_VALUE_ENUM:
		ptr = gcfg_parse_enum(file, ptr, kwd->option.enumtokens, &val);
		break;
	case GCFG_VALUE_NUMBER:
		ptr = gcfg_parse_number(file, ptr, &val, 0);
		break;
	case GCFG_VALUE_SIZE:
		ptr = gcfg_parse_size(file, ptr, &val);
		break;
#ifndef GCFG_DISABLE_VECTOR
	case GCFG_VALUE_VEC2:
		ptr = gcfg_parse_vector(file, ptr, &val, 2);
		break;
	case GCFG_VALUE_VEC3:
		ptr = gcfg_parse_vector(file, ptr, &val, 3);
		break;
	case GCFG_VALUE_VEC4:
		ptr = gcfg_parse_vector(file, ptr, &val, 4);
		break;
#endif
#ifndef GCFG_DISABLE_NETWORK
	case GCFG_VALUE_IPV4:
		ptr = gcfg_parse_ipv4(file, ptr, &val);
		break;
	case GCFG_VALUE_IPV6:
		ptr = gcfg_parse_ipv6(file, ptr, &val);
		break;
	case GCFG_VALUE_MAC:
		ptr = gcfg_parse_mac_addr(file, ptr, &val);
		break;
	case GCFG_VALUE_BANDWIDTH:
		ptr = gcfg_parse_bandwidth(file, ptr, &val);
		break;
#endif
	default:
		file->report_error(file,
				   "[BUG] unknown argument type for '%s'",
				   kwd->name);
		return NULL;
	}

	if (ptr == NULL)
		return NULL;

	*child_out = kwd->set_property(file, parent, &val);
	return skip_space(ptr);
}

static int plain_listing(gcfg_file_t *file, const gcfg_keyword_t *keyword,
			 void *object)
{
	const char *ptr;
	int ret;

	for (;;) {
		ret = file->fetch_line(file);
		if (ret < 0)
			return -1;
		if (ret > 0)
			goto fail_closeing;

		ptr = file->buffer;
		if (!gcfg_is_valid_utf8((const uint8_t *)ptr, strlen(ptr)))
			goto fail_utf8;

		ret = keyword->handle_listing(file, object, ptr);
		if (ret < 0)
			return -1;

		if (ret > 0) {
			ptr = skip_space(ptr);
			if (*ptr != '}')
				goto fail_closeing;

			ptr = skip_space(ptr + 1);
			if (!is_line_end(*ptr))
				goto fail_brace_extra;
			break;
		}
	}
	return 0;
fail_closeing:
	file->report_error(file, "missing '}' after listing");
	return -1;
fail_utf8:
	file->report_error(file, "encoding error (expected UTF-8)");
	return -1;
fail_brace_extra:
	file->report_error(file, "'{' or '}' must be followed by a line break");
	return -1;
}

static int parse(gcfg_file_t *file, const gcfg_keyword_t *keywords,
		 void *parent, unsigned int level)
{
	const gcfg_keyword_t *kwd;
	const char *ptr;
	bool have_args;
	void *child;
	int ret;

	for (;;) {
		ret = file->fetch_line(file);
		if (ret < 0)
			return -1;
		if (ret > 0) {
			if (level != 0)
				goto fail_closeing;
			break;
		}

		if (!gcfg_is_valid_utf8((const uint8_t *)file->buffer,
					strlen(file->buffer))) {
			goto fail_utf8;
		}

		kwd = NULL;
		child = NULL;
		ptr = skip_space(file->buffer);

		if ((*ptr >= 'a' && *ptr <= 'z') ||
		    (*ptr >= 'A' && *ptr <= 'Z')) {
			ptr = find_keyword(file, keywords, ptr, &kwd);
			if (ptr == NULL)
				return -1;

			have_args = !is_line_end(*ptr) && *ptr != '{' &&
				    *ptr != '}';

			if (!have_args && kwd->arg != GCFG_VALUE_NONE)
				goto fail_missing_arg;

			if (have_args && kwd->arg == GCFG_VALUE_NONE)
				goto fail_have_arg;

			ptr = apply_arg(file, kwd, ptr, parent, &child);
			if (ptr == NULL || child == NULL)
				return -1;

			if (*ptr == '{') {
				ptr = skip_space(ptr + 1);
				if (!is_line_end(*ptr))
					goto fail_brace_extra;
				if (kwd->handle_listing != NULL) {
					if (plain_listing(file, kwd, child))
						return -1;
				} else {
					if (kwd->children == NULL)
						goto fail_children;
					if (parse(file, kwd->children,
						  child, level + 1)) {
						return -1;
					}
				}
			} else if (!is_line_end(*ptr)) {
				goto fail_kwd_extra;
			}

			if (kwd->finalize_object != NULL) {
				if (kwd->finalize_object(file, child))
					return -1;
			}
		} else if (*ptr == '}') {
			if (level == 0)
				goto fail_level;
			ptr = skip_space(ptr + 1);
			if (!is_line_end(*ptr))
				goto fail_brace_extra;
			break;
		} else if (!is_line_end(*ptr)) {
			file->report_error(file, "Unexpected %.6s...", ptr);
			return -1;
		}
	}

	return 0;
fail_closeing:
	file->report_error(file, "missing '}' before end-of-file");
	return -1;
fail_utf8:
	file->report_error(file, "encoding error (expected UTF-8)");
	return -1;
fail_kwd_extra:
	if (kwd->arg == GCFG_VALUE_NONE) {
		file->report_error(file, "'%s' must be folled "
				   "by a line break", kwd->name);
	} else {
		file->report_error(file, "'%s <argument>' must be folled "
				   "by a line break", kwd->name);
	}
	return -1;
fail_brace_extra:
	file->report_error(file, "'{' or '}' must be followed by a line break");
	return -1;
fail_level:
	file->report_error(file, "Unexpected '}' outside block");
	return -1;
fail_children:
	file->report_error(file, "Unexpected '{' after %s", kwd->name);
	return -1;
fail_missing_arg:
	file->report_error(file, "Missing argument after %s", kwd->name);
	return -1;
fail_have_arg:
	file->report_error(file, "%s cannot have any arguments", kwd->name);
	return -1;
}

int gcfg_parse_file(gcfg_file_t *file, const gcfg_keyword_t *keywords,
		    void *usr)
{
	return parse(file, keywords, usr, 0);
}
