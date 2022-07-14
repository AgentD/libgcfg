/* SPDX-License-Identifier: ISC */
/*
 * network.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "gcfg.h"


#define STR(x) #x
#define STRVALUE(x) STR(x)

#define CFG_PATH STRVALUE(CFGPATH)


typedef struct switch_t switch_t;
typedef struct port_t port_t;
typedef struct node_t node_t;
typedef struct network_t network_t;

typedef enum {
	PORT_NOT_CONNECTED = 0,
	PORT_CONNECTED_TO_PORT,
	PORT_CONNECTED_TO_SWITCH,
} PORT_CONNECTION_TYPE;

struct switch_t {
	switch_t *next;
	size_t id;

	network_t *owner;
	port_t *ports;
};

struct port_t {
	port_t *owner_next;
	port_t *switch_next;

	node_t *owner;
	char *name;
	gcfg_value_t ip;

	PORT_CONNECTION_TYPE con_type;
	uint64_t outlimit;

	union {
		switch_t *sw;
		port_t *port;
	} connected;
};

struct node_t {
	node_t *next;

	network_t *owner;
	char *name;
	size_t port_count;
	port_t *ports;
	port_t *nat_port;
	bool forwarding;

	gcfg_value_t gateway;
};

struct network_t {
	node_t *nodes;
	size_t num_nodes;

	switch_t *switches;
	size_t num_switches;
};

/********************************** helpers **********************************/

static void network_cleanup(network_t *net)
{
	switch_t *sw;
	node_t *node;
	port_t *p;

	while (net->switches != NULL) {
		sw = net->switches;
		net->switches = sw->next;

		free(sw);
	}

	while (net->nodes != NULL) {
		node = net->nodes;
		net->nodes = node->next;

		while (node->ports != NULL) {
			p = node->ports;
			node->ports = p->owner_next;

			free(p->name);
			free(p);
		}

		free(node->name);
		free(node);
	}
}

static node_t *node_by_name(network_t *net, const char *name, size_t nlen)
{
	node_t *it;

	for (it = net->nodes; it != NULL; it = it->next) {
		if (strncmp(it->name, name, nlen) != 0)
			continue;

		if (it->name[nlen] == '\0')
			return it;
	}

	return NULL;
}

static port_t *port_by_name(node_t *node, const char *name)
{
	port_t *it;

	for (it = node->ports; it != NULL; it = it->owner_next) {
		if (strcmp(it->name, name) == 0)
			return it;
	}

	return NULL;
}

static port_t *port_from_global_name(gcfg_file_t *file, network_t *net,
				     const char *str)
{
	const char *dot = strchr(str, '.');
	node_t *node;
	port_t *port;

	if (dot == NULL || dot == str || dot[1] == '\0') {
		file->report_error(file, "Cannot resolve '%s', "
				   "expected '<node>.<port>'", str);
		return NULL;
	}

	node = node_by_name(net, str, (size_t)(dot - str));
	if (node == NULL) {
		file->report_error(file, "Cannot find node '%.*s'",
				   (int)(dot - str), str);
		return NULL;
	}

	port = port_by_name(node, dot + 1);
	if (port == NULL) {
		file->report_error(file,
				   "Cannot find port '%s' on node '%.*s'",
				   dot + 1, (int)(str - dot), str);
		return NULL;
	}

	return port;
}

/************************* config parsing functions **************************/

static void *node_port_connect_cb(gcfg_file_t *file, void *parent,
				  const gcfg_value_t *value)
{
	const char *str = value->data.string;
	port_t *port = parent;
	network_t *net = port->owner->owner;
	port_t *remote;

	if (port->con_type != PORT_NOT_CONNECTED) {
		file->report_error(file, "Port '%s' on '%s' is already "
				   "connected elsewhere", port->name,
				   port->owner->name);
		return NULL;
	}

	remote = port_from_global_name(file, net, str);
	if (remote == NULL)
		return NULL;

	if (remote->con_type != PORT_NOT_CONNECTED) {
		file->report_error(file, "Port '%s' is already connected "
				   "elsewhere", str);
		return NULL;
	}

	port->con_type = PORT_CONNECTED_TO_PORT;
	remote->con_type = PORT_CONNECTED_TO_PORT;

	port->connected.port = remote;
	remote->connected.port = port;
	return port;
}

static void *node_port_ip_cb(gcfg_file_t *file, void *parent,
			     const gcfg_value_t *ip)
{
	port_t *port = parent;
	(void)file;

	port->ip = *ip;
	return port;
}

static void *node_port_outlimit(gcfg_file_t *file, void *parent,
				const gcfg_value_t *bw)
{
	port_t *port = parent;
	(void)file;

	port->outlimit = bw->data.bandwidth;
	return port;
}

static void *node_port_cb(gcfg_file_t *file, void *parent,
			  const gcfg_value_t *value)
{
	const char *name = value->data.string;
	node_t *node = parent;
	port_t *port;

	if (port_by_name(node, name) != NULL) {
		file->report_error(file, "Node '%s' already has a "
				   "port named '%s'", node->name, name);
		return NULL;
	}

	port = calloc(1, sizeof(*port));
	if (port == NULL)
		goto fail;

	port->name = strdup(name);
	if (port->name == NULL) {
		free(port);
		goto fail;
	}

	port->owner = node;
	port->owner_next = node->ports;
	node->ports = port;
	node->port_count += 1;
	return port;
fail:
	file->report_error(file, "creating node port: %s", strerror(errno));
	return NULL;
}

static void *node_default_gw_cb(gcfg_file_t *file, void *parent,
				const gcfg_value_t *ip)
{
	node_t *node = parent;
	(void)file;

	node->gateway = *ip;
	return node;
}

static void *node_forwarding_cb(gcfg_file_t *file, void *parent,
				const gcfg_value_t *val)
{
	node_t *node = parent;
	(void)file;

	node->forwarding = val->data.boolean;
	return node;
}

static void *node_nat_cb(gcfg_file_t *file, void *parent,
			 const gcfg_value_t *value)
{
	const char *out_if = value->data.string;
	node_t *node = parent;
	port_t *port;

	port = port_by_name(node, out_if);
	if (port == NULL) {
		file->report_error(file, "Unknown port '%s'", out_if);
		return NULL;
	}

	node->nat_port = port;
	return node;
}

static void *switch_port_cb(gcfg_file_t *file, void *parent,
			    const gcfg_value_t *value)
{
	const char *str = value->data.string;
	switch_t *sw = parent;
	network_t *net = sw->owner;
	port_t *port;

	port = port_from_global_name(file, net, str);
	if (port == NULL)
		return NULL;

	if (port->con_type != PORT_NOT_CONNECTED) {
		file->report_error(file, "Port '%s' is already "
				   "connected elsewhere", str);
		return NULL;
	}

	port->con_type = PORT_CONNECTED_TO_SWITCH;
	port->connected.sw = sw;

	port->switch_next = sw->ports;
	sw->ports = port;
	return sw;
}

static void *node_cb(gcfg_file_t *file, void *parent,
		     const gcfg_value_t *value)
{
	const char *name = value->data.string;
	network_t *net = parent;
	node_t *node;

	if (node_by_name(net, name, strlen(name)) != NULL) {
		file->report_error(file, "Node %s already exists!", name);
		return NULL;
	}

	node = calloc(1, sizeof(*node));
	if (node == NULL)
		goto fail;

	node->name = strdup(name);
	if (node->name == NULL) {
		free(node);
		goto fail;
	}

	node->owner = net;
	node->next = net->nodes;
	net->nodes = node;

	net->num_nodes += 1;
	return node;
fail:
	file->report_error(file, "creating node: %s", strerror(errno));
	return NULL;
}

static int node_finalize(gcfg_file_t *file, void *obj)
{
	node_t *node = obj;

	if (node->port_count == 0) {
		file->report_error(file, "Node %s has no ports!", node->name);
		return -1;
	}

	return 0;
}

static void *switch_cb(gcfg_file_t *file, void *parent,
		       const gcfg_value_t *value)
{
	network_t *net = parent;
	switch_t *sw;
	(void)value;

	sw = calloc(1, sizeof(*sw));
	if (sw == NULL) {
		file->report_error(file, "creating switch: %s",
				   strerror(errno));
		return NULL;
	}

	sw->owner = net;
	sw->id = net->num_switches++;
	sw->next = net->switches;
	net->switches = sw;
	return sw;
}

static int switch_finalize(gcfg_file_t *file, void *obj)
{
	switch_t *sw = obj;

	if (sw->ports == NULL) {
		file->report_error(file, "Tried to create a switch "
				   "without any connections!");
		return -1;
	}

	return 0;
}


GCFG_BEGIN_KEYWORDS(kw_node_port)
	GCFG_KEYWORD_IPV4("ipv4", NULL, node_port_ip_cb, NULL),
	GCFG_KEYWORD_STRING("connect", NULL, node_port_connect_cb, NULL),
	GCFG_KEYWORD_BANDWIDTH("outlimit", NULL, node_port_outlimit, NULL),
GCFG_END_KEYWORDS();

GCFG_BEGIN_KEYWORDS(kw_node)
	GCFG_KEYWORD_STRING("port", kw_node_port, node_port_cb, NULL),
	GCFG_KEYWORD_IPV4("defaultgw", NULL, node_default_gw_cb, NULL),
	GCFG_KEYWORD_BOOL("forwarding", NULL, node_forwarding_cb, NULL),
	GCFG_KEYWORD_STRING("nat", NULL, node_nat_cb, NULL),
GCFG_END_KEYWORDS();

GCFG_BEGIN_KEYWORDS(kw_switch)
	GCFG_KEYWORD_STRING("port", NULL, switch_port_cb, NULL),
GCFG_END_KEYWORDS();

GCFG_BEGIN_KEYWORDS(kw_network)
	GCFG_KEYWORD_STRING("node", kw_node, node_cb, node_finalize),
	GCFG_KEYWORD_NO_ARG("switch", kw_switch, switch_cb, switch_finalize),
GCFG_END_KEYWORDS();

int main(void)
{
	gcfg_file_t *file = gcfg_file_open(CFG_PATH);
	network_t net;
	switch_t *sw;
	node_t *n;
	port_t *p;

	memset(&net, 0, sizeof(net));

	if (file == NULL)
		return EXIT_FAILURE;

	if (gcfg_parse_file(file, kw_network, &net)) {
		gcfg_file_close(file);
		network_cleanup(&net);
		return EXIT_FAILURE;
	}

	gcfg_file_close(file);


	printf("Network has %zu nodes.\n", net.num_nodes);
	printf("Network has %zu switches.\n", net.num_switches);

	for (n = net.nodes; n != NULL; n = n->next) {
		printf("Node %s:\n", n->name);

		printf("\tGateway: %u.%u.%u.%u/%u\n",
		       (n->gateway.data.ipv4 >> 24) & 0xFF,
		       (n->gateway.data.ipv4 >> 16) & 0xFF,
		       (n->gateway.data.ipv4 >> 8) & 0xFF,
		       n->gateway.data.ipv4 & 0xFF,
		       n->gateway.cidr_mask);

		if (n->forwarding)
			printf("\tForwarding is enabled\n");

		if (n->nat_port != NULL) {
			printf("\tNAT forwarding on port %s\n",
			       n->nat_port->name);
		}

		printf("\tHas %zu ports:\n", n->port_count);

		for (p = n->ports; p != NULL; p = p->owner_next) {
			printf("\t\t%s %u.%u.%u.%u/%u", p->name,
			       (p->ip.data.ipv4 >> 24) & 0xFF,
			       (p->ip.data.ipv4 >> 16) & 0xFF,
			       (p->ip.data.ipv4 >> 8) & 0xFF,
			       p->ip.data.ipv4 & 0xFF,
			       p->ip.cidr_mask);

			if (p->outlimit > 0)
				printf(" limit %lu bps", p->outlimit);

			if (p->con_type == PORT_CONNECTED_TO_PORT) {
				printf(" -> %s, port %s\n",
				       p->connected.port->owner->name,
				       p->connected.port->name);
			} else if (p->con_type == PORT_CONNECTED_TO_SWITCH) {
				printf(" -> switch %zu\n",
				       p->connected.sw->id);
			} else {
				putchar('\n');
			}
		}
	}

	for (sw = net.switches; sw != NULL; sw = sw->next) {
		printf("Switch %zu:\n", sw->id);

		for (p = sw->ports; p != NULL; p = p->switch_next) {
			printf("\t%s, port %s\n", p->owner->name, p->name);
		}
	}

	network_cleanup(&net);
	return EXIT_SUCCESS;
}
