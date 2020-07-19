/* SPDX-License-Identifier: ISC */
/*
 * util.h
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#ifndef UTIL_H
#define UTIL_H

#include "gcfg.h"

const char *gcfg_dec_num(gcfg_file_t *f, const char *str,
			 uint64_t *out, uint64_t max);

const char *gcfg_ipv4address(gcfg_file_t *f, const char *str, uint32_t *out);

const char *gcfg_parse_ipv4(gcfg_file_t *f, const char *in,
			    gcfg_ip_addr_t *ret);

const char *gcfg_parse_ipv6(gcfg_file_t *f, const char *in,
			    gcfg_ip_addr_t *ret);

const char *gcfg_parse_mac_addr(gcfg_file_t *f, const char *in,
				uint32_t *vendor, uint32_t *device);

const char *gcfg_parse_bandwidth(gcfg_file_t *f, const char *in, uint64_t *ret);

const char *gcfg_parse_number(gcfg_file_t *f, const char *in,
			      gcfg_number_t *num);

const char *gcfg_parse_boolean(gcfg_file_t *f, const char *in, int *out);

const char *gcfg_parse_vector(gcfg_file_t *f, const char *in,
			      int count, gcfg_number_t *out);

const char *gcfg_parse_enum(gcfg_file_t *f, const char *in,
			    const gcfg_enum_t *tokens, int *out);

const char *gcfg_parse_size(gcfg_file_t *f, const char *in, uint64_t *ret);

const char *gcfg_parse_string(gcfg_file_t *f, const char *in, char *out);

bool gcfg_is_valid_cp(uint32_t cp);

bool gcfg_is_valid_utf8(const uint8_t *str, size_t len);

int gcfg_xdigit(int c);

#ifdef BUILD_TEST
void dummy_file_init(gcfg_file_t *f, const char *line);

void dummy_file_cleanup(gcfg_file_t *f);
#endif

#endif /* UTIL_H */
