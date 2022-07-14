/* SPDX-License-Identifier: ISC */
/*
 * parse_mac_addr.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"
#include "test.h"

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
	gcfg_value_t net;
	char buffer[64];
	const char *ret;

	ret = gcfg_parse_mac_addr(df, testvec[i].in, &net);

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

	if (net.type != GCFG_VALUE_MAC) {
		fprintf(stderr, "Address %zu (%s) has a wrong type (%u)\n",
			i, testvec[i].in, net.type);
		print_mac(buffer, testvec[i].vendor, testvec[i].device);
	}

	if (net.flags != 0) {
		fprintf(stderr, "Address %zu (%s) has a flags set (%X)\n",
			i, testvec[i].in, net.flags);
		print_mac(buffer, testvec[i].vendor, testvec[i].device);
	}

	if (testvec[i].vendor != net.data.mac.vendor ||
	    testvec[i].device != net.data.mac.device) {
		fprintf(stderr, "Mismatch for %zu\n", i);
		print_mac(buffer, testvec[i].vendor, testvec[i].device);
		fprintf(stderr, "Expected: %s\n", buffer);
		print_mac(buffer, net.data.mac.vendor, net.data.mac.device);
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
