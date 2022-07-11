/* SPDX-License-Identifier: ISC */
/*
 * parse_uri.c
 *
 * Copyright (C) 2022 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"
#include "test.h"

static const struct {
	const char *str;
	int result;
	const char *scheme;
	const char *userinfo;
	const char *host;
	const char *path;
	const char *query;
	const char *fragment;
	uint16_t port;
	uint16_t flags;
} testvec[] = {
	{
		"https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top",
		0,
		"https",
		"john.doe",
		"www.example.com",
		"/forum/questions",
		"tag=networking&order=newest",
		"top",
		123,
		GCFG_URI_HAS_PORT | GCFG_URI_HOST_NAME,
	}, {
		"ldap://[2001:db8::7]/c=GB?objectClass?one",
		0,
		"ldap",
		NULL,
		"2001:db8::7",
		"/c=GB",
		"objectClass?one",
		NULL,
		0,
		GCFG_URI_HOST_IPV6,
	}, {
		"HTTP://[2001:db8::7]:1337",
		0,
		"http",
		NULL,
		"2001:db8::7",
		"/",
		NULL,
		NULL,
		1337,
		GCFG_URI_HAS_PORT | GCFG_URI_HOST_IPV6,
	}, {
		"mailto:John.Doe@example.com",
		0,
		"mailto",
		NULL,
		NULL,
		"John.Doe@example.com",
		NULL,
		NULL,
		0,
		0,
	}, {
		"news:comp.infosystems.www.servers.unix",
		0,
		"news",
		NULL,
		NULL,
		"comp.infosystems.www.servers.unix",
		NULL,
		NULL,
		0,
		0,
	}, {
		"tel:+1-816-555-1212",
		0,
		"tel",
		NULL,
		NULL,
		"+1-816-555-1212",
		NULL,
		NULL,
		0,
		0,
	}, {
		"telnet://192.0.2.16:80/",
		0,
		"telnet",
		NULL,
		"192.0.2.16",
		"/",
		NULL,
		NULL,
		80,
		GCFG_URI_HAS_PORT | GCFG_URI_HOST_IPV4,
	}
};

static int match_str(const char *ref, const char *cmp)
{
	if (cmp == NULL)
		return ref != NULL;

	return (ref == NULL) || strcmp(ref, cmp);
}

static void test_case(gcfg_file_t *df, size_t i)
{
	char buffer[128];
	const char *ret;
	gcfg_uri_t uri;

	ret = gcfg_parse_uri(df, testvec[i].str, buffer, &uri);

	if (ret == NULL) {
		if (testvec[i].result == 0) {
			fprintf(stderr, "URI `%s` was not accepted!",
				testvec[i].str);
			exit(EXIT_FAILURE);
		}
		return;
	}

	if (testvec[i].result != 0) {
		fprintf(stderr, "URI `%s` was accepted!\n", testvec[i].str);
		exit(EXIT_FAILURE);
	}

	if (uri.scheme == NULL || strcmp(testvec[i].scheme, uri.scheme) != 0)
		goto fail_scheme;

	if (match_str(testvec[i].userinfo, uri.userinfo))
		goto fail_uinfo;

	if (match_str(testvec[i].host, uri.host))
		goto fail_host;

	if (testvec[i].flags & GCFG_URI_HAS_PORT) {
		if (!(uri.flags & GCFG_URI_HAS_PORT))
			goto fail_port;
		if (testvec[i].port != uri.port)
			goto fail_port;
	} else {
		if (uri.flags & GCFG_URI_HAS_PORT)
			goto fail_no_port;
	}

	if (match_str(testvec[i].path, uri.path))
		goto fail_path;

	if (match_str(testvec[i].query, uri.query))
		goto fail_query;

	if (match_str(testvec[i].fragment, uri.fragment))
		goto fail_frag;

	if (uri.flags != testvec[i].flags) {
		fprintf(stderr, "URI `%s` has mismatching flags!\n",
			testvec[i].str);
		exit(EXIT_FAILURE);
	}

	return;
fail_scheme:
	fprintf(stderr, "URI `%s` scheme was extracted as `%s`!\n",
		testvec[i].str, uri.scheme);
	exit(EXIT_FAILURE);
fail_uinfo:
	fprintf(stderr, "URI `%s` userinfo was extracted as `%s`!\n",
		testvec[i].str, uri.userinfo);
	exit(EXIT_FAILURE);
fail_host:
	fprintf(stderr, "URI `%s` host was extracted as `%s`!\n",
		testvec[i].str, uri.host);
	exit(EXIT_FAILURE);
fail_no_port:
	fprintf(stderr, "URI `%s` port number was not recognized!\n",
		testvec[i].str);
	exit(EXIT_FAILURE);
fail_port:
	fprintf(stderr, "URI `%s` port number was extracted as `%u`!\n",
		testvec[i].str, (unsigned int)uri.port);
	exit(EXIT_FAILURE);
fail_path:
	fprintf(stderr, "URI `%s` path was extracted as `%s`!\n",
		testvec[i].str, uri.path);
	exit(EXIT_FAILURE);
fail_query:
	fprintf(stderr, "URI `%s` query was extracted as `%s`!\n",
		testvec[i].str, uri.query);
	exit(EXIT_FAILURE);
fail_frag:
	fprintf(stderr, "URI `%s` fragment was extracted as `%s`!\n",
		testvec[i].str, uri.fragment);
	exit(EXIT_FAILURE);
}

int main(void)
{
	gcfg_file_t df;
	size_t i;

	for (i = 0; i < sizeof(testvec) / sizeof(testvec[0]); ++i) {
		dummy_file_init(&df, testvec[i].str);
		test_case(&df, i);
		dummy_file_cleanup(&df);
	}

	return EXIT_SUCCESS;
}
