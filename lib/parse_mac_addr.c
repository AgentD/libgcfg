/* SPDX-License-Identifier: ISC */
/*
 * parse_mac_addr.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "gcfg.h"

const char *gcfg_parse_mac_addr(gcfg_file_t *f, const char *in,
				uint32_t *vendor, uint32_t *device)
{
	const char *start = in;
	uint64_t val = 0;
	int i;

	for (i = 0; i < 6; ++i) {
		if (i > 0 && *(in++) != ':')
			goto fail;

		if (gcfg_xdigit(in[0]) < 0 || gcfg_xdigit(in[1]) < 0)
			goto fail;

		val <<= 8;
		val |= (uint64_t)(gcfg_xdigit(in[0]) << 4);
		val |= (uint64_t)gcfg_xdigit(in[1]);
		in += 2;
	}

	*vendor = (val >> 24) & 0x00FFFFFF;
	*device =  val        & 0x00FFFFFF;
	return in;
fail:
	f->report_error(f, "invalid MAC address '%.*s...'",
			(int)(in - start + 1), start);
	return NULL;
}

#ifdef BUILD_TEST
static const struct {
	const char *in;
	uint32_t vendor;
	uint32_t device;
	int ret;
} testvec[] = {
	{ "BA:D0:CA:FE:BA:BE", 0x00BAD0CA, 0x00FEBABE, 0 },
	{ "DE:AD:00:BE:EF:00", 0x00DEAD00, 0x00BEEF00, 0 },
	{ "DE:AD:BE:EF:00:", 0, 0, -1 },
	{ "DE:AD:BE:EF:00", 0, 0, -1 },
	{ "8:BA:D:F0:D:0", 0, 0, -1 },
};


static void print_mac(char *buffer, uint32_t vendor, uint32_t device)
{
	sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
		(vendor >> 16) & 0x00FF, (vendor >> 8) & 0x00FF,
		vendor & 0x00FF, (device >> 16) & 0x00FF,
		(device >> 8) & 0x00FF, device & 0x00FF);
}

static void test_case(gcfg_file_t *df, size_t i)
{
	uint32_t vendor, device;
	char buffer[64];
	const char *ret;

	ret = gcfg_parse_mac_addr(df, testvec[i].in, &vendor, &device);

	if ((ret == NULL && testvec[i].ret == 0) ||
	    (ret != NULL && testvec[i].ret != 0)) {
		fprintf(stderr, "Wrong return status for %zu\n", i);
		fprintf(stderr, "Input: '%s' was %s\n",
			testvec[i].in,
			ret == NULL ? "not accepted" : "accepted");
		exit(EXIT_FAILURE);
	}

	if (ret == NULL)
		return;

	if (testvec[i].vendor != vendor || testvec[i].device != device) {
		fprintf(stderr, "Mismatch for %zu\n", i);
		print_mac(buffer, testvec[i].vendor, testvec[i].device);
		fprintf(stderr, "Expected: %s\n", buffer);
		print_mac(buffer, vendor, device);
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
