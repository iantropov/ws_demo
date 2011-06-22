/*
 * driver.c
 *
 *  Created on: Mar 21, 2011
 *      Author: ant
 */

#include <stdlib.h>
#include <string.h>
#include <event.h>
#include <json_rpc/json.h>
#include <json_rpc/json_parser.h>

#include "util.h"
#include "log.h"
#include "driver.h"

static struct device *devices[255] = {0};

struct track {
	struct event ev;

	int id;
	int timeout;

	void (*cb)(int id, void *);
	void *arg;
};

struct device {
	int status;
	char *info;
	char *type;
	int id;

	struct track *tr;
};

static void track_wrap(int fd, short what, void *arg)
{
	struct track *tr = (struct track *)arg;

	tr->cb(tr->id, tr->arg);

	struct timeval tv = {0, tr->timeout};
	if (event_add(&tr->ev, &tv) == -1) {
		free(tr);
	}
}

static struct track *track_new(int id, void (*cb)(int, void *arg), void *arg, int timeout)
{
	struct track *tr = (struct track *)calloc(1, sizeof(struct track));
	if (tr == NULL)
		return NULL;

	tr->cb = cb;
	tr->arg = arg;
	tr->id = id;
	tr->timeout = timeout;

	event_set(&tr->ev, -1, EV_TIMEOUT, track_wrap, tr);

	struct timeval tv = {0, tr->timeout};
	if (event_add(&tr->ev, &tv) == -1) {
		free(tr);
		return NULL;
	}

	return tr;
}

static void track_free(struct track *tr)
{
	if (tr == NULL)
		return;

	event_del(&tr->ev);
	free(tr);
}

static struct device *device_new(int id, char *info, char *type)
{
	if (id > DEVICES_LIST_SIZE || id < 0 || info == NULL)
		return NULL;

	struct device *dev = (struct device *)calloc(1, sizeof(struct device));
	if (dev == NULL)
		return NULL;

	dev->type = strdup(type);
	if (dev->type == NULL) {
		free(dev);
		return NULL;
	}

	dev->info = strdup(info);
	if (dev->info == NULL) {
		free(dev->type);
		free(dev);
		return NULL;
	}

	dev->id = id;
	dev->status = MIN_STATUS_VALUE;

	return dev;
}

static void device_free(struct device *dev)
{
	if (dev == NULL)
		return;

	free(dev->info);
	free(dev->type);

	if (dev->tr != NULL)
		track_free(dev->tr);

	free(dev);
}

int driver_get_device_status(int id)
{
	if (id < 0 || id > DEVICES_LIST_SIZE || devices[id] == NULL)
		return -1;

	return devices[id]->status;
}

void driver_set_device_status(int id, int value)
{
	if (	id < 0 || id > DEVICES_LIST_SIZE ||
			value < MIN_STATUS_VALUE || value > MAX_STATUS_VALUE ||
			devices[id] == NULL)
		return;

	devices[id]->status = value;
}

char *driver_get_device_info(int id)
{
	if (id < 0 || id > DEVICES_LIST_SIZE || devices[id] == NULL)
		return NULL;

	return devices[id]->info;
}

char *driver_get_device_type(int id)
{
	if (id < 0 || id > DEVICES_LIST_SIZE || devices[id] == NULL)
		return NULL;

	return devices[id]->type;
}

void driver_destroy()
{
	int i;
	for (i = 0; i < DEVICES_LIST_SIZE; i++)
		device_free(devices[i]);
}

int driver_start_track(int id, void (*cb)(int id, void *), void *arg, int timeout)
{
	if (id < 0 || id > DEVICES_LIST_SIZE || devices[id] == NULL || devices[id]->tr != NULL)
		return -1;

	struct device *dev = devices[id];
	dev->tr = track_new(id, cb, arg, timeout);
	if (dev->tr == NULL)
		return -1;

	return 0;
}

void driver_stop_track(int id)
{
	if (id < 0 || id > DEVICES_LIST_SIZE || devices[id] == NULL || devices[id]->tr == NULL)
		return;

	struct device *dev = devices[id];

	track_free(dev->tr);
	dev->tr = NULL;
}

void driver_stop_track_all()
{
	int i;
	for (i = 0; i < DEVICES_LIST_SIZE; i++) {
		driver_stop_track(i);
	}
}

int driver_load_devices(char *path)
{
	char *file_content = get_file_content(path);
	if (file_content == NULL) {
		log_warn("driver_init failed : couldn`t load devices\n");
		return -1;
	}

	struct json_object *devices_json = json_parser_parse(file_content);
	if (devices == NULL || json_type(devices_json) != json_type_array) {
		log_warn("devices file malformed\n");
		return -1;
	}


	int result = 0, i, len = json_array_length(devices_json), id;
	struct json_object *device_json;
	char *type, *info;
	for (i = 0; i < len; i++) {
		device_json = json_array_get(devices_json, i);

		id = json_int_get(json_object_get(device_json, "id"));
		if (id < 0 || id > DEVICES_LIST_SIZE || devices[id] != NULL) {
			log_warn("wrong id\n");
			driver_destroy();
			goto exit;
		}

		info = json_string_get(json_object_get(device_json, "info"));
		type = json_string_get(json_object_get(device_json, "type"));

		devices[id] = device_new(id, info, type);
		if (devices[id] == NULL) {
			log_warn("init device error\n");
			driver_destroy();
			goto exit;
		}
	}

exit:
	json_ref_put(devices_json);
	free(file_content);

	return result;
}
