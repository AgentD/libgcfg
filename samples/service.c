/* SPDX-License-Identifier: ISC */
/*
 * service.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

#include "gcfg.h"


#define STR(x) #x
#define STRVALUE(x) STR(x)

#define CFG_PATH STRVALUE(CFGPATH)

typedef enum {
	SERVICE_TYPE_WAIT = 1,
	SERVICE_TYPE_ONCE,
	SERVICE_TYPE_RESTART,

	SERVICE_TYPE_MIN = SERVICE_TYPE_WAIT,
	SERVICE_TYPE_MAX = SERVICE_TYPE_RESTART,
} E_SERVICE_TYPE;

typedef enum {
	SERVICE_TARGET_BOOT = 1,
	SERVICE_TARGET_REBOOT,
	SERVICE_TARGET_SHUTDOWN,
	SERVICE_TARGET_CTRL_ALT_DEL,

	SERVICE_TARGET_MIN = SERVICE_TARGET_BOOT,
	SERVICE_TARGET_MAX = SERVICE_TARGET_CTRL_ALT_DEL,
} E_SERVICE_TARGET;

typedef struct service_dependency_t {
	struct service_dependency_t *next;
	char *name;
} service_dependency_t;

typedef struct service_exec_t {
	struct service_exec_t *next;
	char *raw;
} service_exec_t;

typedef struct exec_block_t {
	struct exec_block_t *next;
	char *interpreter;
	service_exec_t *list;
} exec_block_t;

typedef struct {
	char *description;

	service_dependency_t *before;
	service_dependency_t *after;
	exec_block_t *exec;

	int type;
	int target;
} service_t;

/********************************** helpers **********************************/

static void service_cleanup(service_t *svc)
{
	service_dependency_t *dep;
	service_exec_t *exec;
	exec_block_t *eblk;

	while (svc->before != NULL) {
		dep = svc->before;
		svc->before = dep->next;

		free(dep->name);
		free(dep);
	}

	while (svc->after != NULL) {
		dep = svc->after;
		svc->after = dep->next;

		free(dep->name);
		free(dep);
	}

	while (svc->exec != NULL) {
		eblk = svc->exec;
		svc->exec = eblk->next;

		while (eblk->list != NULL) {
			exec = eblk->list;
			eblk->list = exec->next;

			free(exec->raw);
			free(exec);
		}

		free(eblk);
	}

	free(svc->description);
	free(svc);
}

static service_dependency_t *find_after_dependency(service_t *svc,
						   const char *name)
{
	service_dependency_t *dep;

	for (dep = svc->after; dep != NULL; dep = dep->next) {
		if (strcmp(dep->name, name) == 0)
			return dep;
	}

	return NULL;
}

static service_dependency_t *find_before_dependency(service_t *svc,
						    const char *name)
{
	service_dependency_t *dep;

	for (dep = svc->before; dep != NULL; dep = dep->next) {
		if (strcmp(dep->name, name) == 0)
			return dep;
	}

	return NULL;
}

/************************* config parsing functions **************************/

static void *svc_description_cb(gcfg_file_t *file, void *parent,
				const char *string)
{
	service_t *svc = parent;
	(void)file;

	free(svc->description);
	svc->description = strdup(string);
	return svc;
}

static void *svc_target_cb(gcfg_file_t *file, void *svc, int value)
{
	(void)file;
	((service_t *)svc)->type = value;
	return svc;
}

static void *svc_type_cb(gcfg_file_t *file, void *svc, int value)
{
	(void)file;
	((service_t *)svc)->target = value;
	return svc;
}

static void *svc_after_cb(gcfg_file_t *file, void *parent,
			  const char *string)
{
	service_dependency_t *dep;
	service_t *svc = parent;

	if (find_before_dependency(svc, string) != NULL)
		goto fail_before;

	if (find_after_dependency(svc, string) != NULL)
		return svc;

	dep = calloc(1, sizeof(*dep));
	if (dep == NULL)
		goto fail_alloc;

	dep->name = strdup(string);
	dep->next = svc->after;
	svc->after = dep;
	return svc;
fail_alloc:
	file->report_error(file, "out of memory");
	return NULL;
fail_before:
	file->report_error(file, "%s: already specified as "
			   "'before' dependency", string);
	return NULL;
}

static void *svc_before_cb(gcfg_file_t *file, void *parent, const char *string)
{
	service_dependency_t *dep;
	service_t *svc = parent;

	if (find_after_dependency(svc, string) != NULL)
		goto fail_after;

	if (find_before_dependency(svc, string) != NULL)
		return svc;

	dep = calloc(1, sizeof(*dep));
	if (dep == NULL)
		goto fail_alloc;

	dep->name = strdup(string);
	dep->next = svc->before;
	svc->before = dep;
	return svc;
fail_alloc:
	file->report_error(file, "out of memory");
	return NULL;
fail_after:
	file->report_error(file, "%s: already specified as "
			   "'after' dependency", string);
	return NULL;
}

static void *svc_begin_exec_block(gcfg_file_t *file, void *object,
				  const char *string)
{
	service_t *svc = object;
	exec_block_t *exec, *it;

	exec = calloc(1, sizeof(*exec));
	if (exec == NULL) {
		file->report_error(file, "out of memory");
		return NULL;
	}

	for (it = svc->exec; it != NULL && it->next != NULL; it = it->next)
		;

	if (it == NULL) {
		svc->exec = exec;
	} else {
		it->next = exec;
	}

	exec->next = NULL;
	exec->interpreter = strdup(string);
	return exec;
}

static int svc_exec_line(gcfg_file_t *file, void *object,
			 const char *line)
{
	exec_block_t *blk = object;
	service_exec_t *exec, *it;

	while (isspace(*line))
		++line;

	if (*line == '}')
		return 1;

	if (*line == '\0' || *line == '#')
		return 0;

	exec = calloc(1, sizeof(*exec));
	if (exec == NULL)
		goto fail;

	exec->raw = strdup(line);
	if (exec->raw == NULL) {
		free(exec);
		goto fail;
	}

	for (it = blk->list; it != NULL && it->next != NULL; it = it->next)
		;

	if (it == NULL) {
		blk->list = exec;
	} else {
		it->next = exec;
	}
	return 0;
fail:
	file->report_error(file, "out of memory");
	return -1;
}

GCFG_BEGIN_ENUM(target_enum)
	GCFG_ENUM("boot", SERVICE_TARGET_BOOT),
	GCFG_ENUM("reboot", SERVICE_TARGET_REBOOT),
	GCFG_ENUM("shutdown", SERVICE_TARGET_SHUTDOWN),
	GCFG_ENUM("ctrlaltdel", SERVICE_TARGET_CTRL_ALT_DEL),
GCFG_END_ENUM();

GCFG_BEGIN_ENUM(type_enum)
	GCFG_ENUM("wait", SERVICE_TYPE_WAIT),
	GCFG_ENUM("once", SERVICE_TYPE_ONCE),
	GCFG_ENUM("restart", SERVICE_TYPE_RESTART),
GCFG_END_ENUM();

GCFG_BEGIN_KEYWORDS(kw_service)
	GCFG_KEYWORD_STRING("description", NULL, svc_description_cb, NULL),
	GCFG_KEYWORD_ENUM("target", NULL, target_enum, svc_target_cb, NULL),
	GCFG_KEYWORD_ENUM("type", NULL, type_enum, svc_type_cb, NULL),
	GCFG_KEYWORD_STRING("after", NULL, svc_after_cb, NULL),
	GCFG_KEYWORD_STRING("before", NULL, svc_before_cb, NULL),
	{
		.name = "exec",
		.arg = GCFG_ARG_STRING,
		.handle = { .cb_string = svc_begin_exec_block },
		.handle_listing = svc_exec_line,
	},
GCFG_END_KEYWORDS();

/*****************************************************************************/

int main(void)
{
	gcfg_file_t *file = gcfg_file_open(CFG_PATH);
	service_dependency_t *dep;
	service_exec_t *exec;
	exec_block_t *eblk;
	service_t *svc;

	if (file == NULL)
		return EXIT_FAILURE;

	svc = calloc(1, sizeof(*svc));
	if (svc == NULL) {
		file->report_error(file, "out of memory");
		gcfg_file_close(file);
		return EXIT_FAILURE;
	}

	if (gcfg_parse_file(file, kw_service, svc)) {
		gcfg_file_close(file);
		service_cleanup(svc);
		return EXIT_FAILURE;
	}

	gcfg_file_close(file);

	printf("Service description: %s\n", svc->description);
	printf("Service type ID: %d\n", svc->type);
	printf("Service target ID: %d\n", svc->target);

	if (svc->before != NULL) {
		printf("Must be run before:\n");

		for (dep = svc->before; dep != NULL; dep = dep->next) {
			printf("\t%s\n", dep->name);
		}
	}

	if (svc->after != NULL) {
		printf("Must be run after:\n");

		for (dep = svc->after; dep != NULL; dep = dep->next) {
			printf("\t%s\n", dep->name);
		}
	}

	for (eblk = svc->exec; eblk != NULL; eblk = eblk->next) {
		printf("Run the following commands with \"%s\":\n",
		       eblk->interpreter);

		for (exec = eblk->list; exec != NULL; exec = exec->next) {
			printf("\t%s\n", exec->raw);
		}
	}

	service_cleanup(svc);
	return EXIT_SUCCESS;
}
