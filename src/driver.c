/*
 * driver.c
 *
 *  Created on: Mar 21, 2011
 *      Author: ant
 */

#include <stdlib.h>
#include <string.h>
#include <json_rpc/json.h>
#include <json_rpc/json_parser.h>

#include "util.h"
#include "log.h"

#define DEVICES_LIST_SIZE 255
#define MIN_STATUS_VALUE 0
#define MAX_STATUS_VALUE 255
static int device_statuses[DEVICES_LIST_SIZE];
static char *device_infos[DEVICES_LIST_SIZE];

static void init_device_statuses_and_infos()
{
	int i;
	for (i = 0; i < DEVICES_LIST_SIZE; i++) {
		device_statuses[i] = -1;
		device_infos[i] = NULL;
	}
}

int driver_init(char *path)
{
	char *file_content = get_file_content(path);
	if (file_content == NULL) {
		log_warn("driver_init failed : couldn`t load devices\n");
		return -1;
	}

	struct json_object *devices = json_parser_parse(file_content);
	if (devices == NULL || json_type(devices) != json_type_array) {
		log_warn("devices file malformed\n");
		return -1;
	}

	init_device_statuses_and_infos();

	int i, len = json_array_length(devices), id;
	struct json_object *device;
	char *info;
	for (i = 0; i < len; i++) {
		device = json_array_get(devices, i);
		id = json_int_get(json_object_get(device, "id"));
		info = json_string_get(json_object_get(device, "info"));
		device_statuses[id] = MIN_STATUS_VALUE;
		device_infos[id] = strdup(info);
	}

	json_ref_put(devices);
	free(file_content);

	return 0;
}

int driver_get_device_status(int id)
{
	if (id < 0 || id > DEVICES_LIST_SIZE)
		return -1;

	return device_statuses[id];
}

void driver_set_device_status(int id, int value)
{
	if (	id < 0 || id > DEVICES_LIST_SIZE ||
			value < MIN_STATUS_VALUE || value > MAX_STATUS_VALUE ||
			device_statuses[id] == -1)
		return;

	device_statuses[id] = value;
}

char *driver_get_device_info(int id)
{
	if (id < 0 || id > DEVICES_LIST_SIZE)
		return NULL;

	return device_infos[id];
}

void driver_destroy()
{
	int i;
	for (i = 0; i < DEVICES_LIST_SIZE; i++)
		free(device_infos[i]);
}
