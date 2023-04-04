/* SPDX-License-Identifier: ISC */
/*
 * gcfg.h
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#ifndef GCFG_H
#define GCFG_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define GCFG_PRINTF_FUN(fmtidx, elidx) \
	__attribute__ ((format (printf, fmtidx, elidx)))

typedef enum {
	GCFG_VALUE_NONE = 0,
	GCFG_VALUE_IPV4 = 1,
	GCFG_VALUE_IPV6 = 2,
	GCFG_VALUE_MAC = 3,
	GCFG_VALUE_BANDWIDTH = 4,
	GCFG_VALUE_SIZE = 5,
	GCFG_VALUE_ENUM = 6,
	GCFG_VALUE_BOOLEAN = 7,
	GCFG_VALUE_STRING = 8,
	GCFG_VALUE_NUMBER = 9,
	GCFG_VALUE_PERCENTAGE = 10,
	GCFG_VALUE_VEC2 = 11,
	GCFG_VALUE_VEC3 = 12,
	GCFG_VALUE_VEC4 = 13,
	GCFG_VALUE_URI = 14,
} GCFG_VALUE_TYPE;

typedef enum {
	GCFG_NET_ADDR_HAVE_MASK = 0x01,
} GCFG_NET_ADDR_FLAGS;

typedef enum {
	GCFG_URI_HAS_PORT = 0x01,

	GCFG_URI_HOST_IPV4 = 0x02,

	GCFG_URI_HOST_IPV6 = 0x04,

	GCFG_URI_HOST_NAME = 0x08,
} GCFG_URI_FLAGS;

typedef struct {
	union {
		uint32_t ipv4;
		uint16_t ipv6[8];

		struct {
			uint32_t vendor;
			uint32_t device;
		} mac;

		uint64_t bandwidth;

		uint64_t size;

		intptr_t enum_value;

		bool boolean;

		char *string;

		struct {
			int64_t value;
			int32_t exponent;
		} number[4];

		struct {
			char *scheme;
			char *userinfo;
			char *host;
			char *path;
			char *query;
			char *fragment;
			uint16_t port;
		} uri;
	} data;

	uint16_t flags;
	uint8_t cidr_mask;
	uint8_t type;
} gcfg_value_t;

typedef struct {
	const char *name;
	intptr_t value;
} gcfg_enum_t;

typedef struct gcfg_file_t {
	void (*report_error)(struct gcfg_file_t *f, const char *msg, ...)
		GCFG_PRINTF_FUN(2, 3);

	/* Reads a line into the mutable buffer below. Returns > 0 on EOF, < 0
	   on internal error, 0 on success.

	   On success, line break at the end is removed from the line and a
	   null-terminator is added.
	*/
	int (*fetch_line)(struct gcfg_file_t *f);

	/* Mutable buffer holding the current line */
	char *buffer;
} gcfg_file_t;

typedef struct gcfg_keyword_t {
	uint32_t arg;
	uint32_t pad0;

	const char *name;

	union {
		const gcfg_enum_t *enumtokens;
	} option;

	void *(*set_property)(gcfg_file_t *file, void *parent,
			      const gcfg_value_t *value);

	const struct gcfg_keyword_t *children;

	int (*finalize_object)(gcfg_file_t *file, void *child);

	int (*handle_listing)(gcfg_file_t *file, void *child,
			      const char *line);
} gcfg_keyword_t;


#define GCFG_BEGIN_ENUM(name) static const gcfg_enum_t name[] = {

#define GCFG_ENUM(nam, val) { .name = nam, .value = val }

#define GCFG_END_ENUM() { 0, 0 } }


#define GCFG_KEYWORD_BASE(nam, karg, elist, clist, cb, finalize) \
	{ \
		.name = nam, \
		.arg = karg, \
		.option = { .enumtokens = elist, }, \
		.children = clist, \
		.set_property = cb, \
		.finalize_object = finalize, \
	}

#define GCFG_BEGIN_KEYWORDS(listname) \
	static const gcfg_keyword_t listname[] = {

#define GCFG_KEYWORD_NO_ARG(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_NONE, NULL, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_BOOL(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_BOOLEAN, NULL, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_STRING(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_STRING, NULL, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_ENUM(kwdname, childlist, enumlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_ENUM, enumlist, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_NUMBER(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_NUMBER, NULL, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_VEC2(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_VEC2, NULL, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_VEC3(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_VEC3, NULL, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_VEC4(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_VEC4, NULL, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_IPV4(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_IPV4, NULL, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_IPV6(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_IPV6, NULL, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_MAC(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_MAC, NULL, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_BANDWIDTH(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_BANDWIDTH, NULL, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_SIZE(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_SIZE, NULL, childlist, \
			  callback, finalize)

#define GCFG_KEYWORD_URI(kwdname, childlist, callback, finalize) \
	GCFG_KEYWORD_BASE(kwdname, GCFG_VALUE_URI, NULL, childlist, \
			  callback, finalize)

#define GCFG_END_KEYWORDS() \
		{ .name = NULL }, \
	}

#ifdef __cplusplus
extern "C" {
#endif

double gcfg_number_to_double(const gcfg_value_t *num, size_t index);

int gcfg_parse_file(gcfg_file_t *file, const gcfg_keyword_t *keywords,
		    void *usr);

gcfg_file_t *gcfg_file_open(const char *path);

void gcfg_file_close(gcfg_file_t *file);





const char *gcfg_dec_num(gcfg_file_t *f, const char *str,
			 uint64_t *out, uint64_t max);

const char *gcfg_ipv4address(gcfg_file_t *f, const char *str, uint32_t *out);

const char *gcfg_parse_ipv4(gcfg_file_t *f, const char *in, gcfg_value_t *ret);

const char *gcfg_parse_ipv6(gcfg_file_t *f, const char *in, gcfg_value_t *ret);

const char *gcfg_parse_mac_addr(gcfg_file_t *f, const char *in,
				gcfg_value_t *ret);

const char *gcfg_parse_bandwidth(gcfg_file_t *f, const char *in,
				 gcfg_value_t *ret);

const char *gcfg_parse_number(gcfg_file_t *f, const char *in,
			      gcfg_value_t *out, size_t index);

const char *gcfg_parse_boolean(gcfg_file_t *f, const char *in,
			       gcfg_value_t *out);

const char *gcfg_parse_vector(gcfg_file_t *f, const char *in,
			      gcfg_value_t *out, size_t count);

const char *gcfg_parse_enum(gcfg_file_t *f, const char *in,
			    const gcfg_enum_t *tokens, gcfg_value_t *out);

const char *gcfg_parse_size(gcfg_file_t *f, const char *in,
			    gcfg_value_t *ret);

const char *gcfg_parse_string(gcfg_file_t *f, const char *in, char *out);

const char *gcfg_parse_uri(gcfg_file_t *f, const char *in,
			   char *buffer, gcfg_value_t *out);

bool gcfg_is_valid_cp(uint32_t cp);

bool gcfg_is_valid_utf8(const uint8_t *str, size_t len);

int gcfg_xdigit(int c);

#ifdef __cplusplus
}
#endif

#endif /* GCFG_H */
