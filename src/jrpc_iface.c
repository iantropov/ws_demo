/*
 * jrpc_iface.c
 *
 *  Created on: Jun 20, 2011
 *      Author: ant
 */

#include "driver.h"
#include "jrpc_iface.h"
#include "log.h"

#include <json_rpc/json_parser.h>
#include <json_rpc/json_rpc_tt.h>
#include <ws/web_sockets.h>

#define GET_DEVICES_METHOD "get_devices"
#define GET_DEVICE_STATUS_METHOD "get_device_status"
#define SET_DEVICE_STATUS_METHOD "set_device_status"
#define RTDEVICE_START_TRACK_METHOD "rtdevice_start_track"
#define RTDEVICE_STOP_TRACK_METHOD "rtdevice_stop_track"
#define DEVICE_START_TRACK_METHOD "device_start_track"
#define DEVICE_STOP_TRACK_METHOD "device_stop_track"

#define RTDEVICE_TIMEOUT_TRACK 1000
#define DEVICE_TIMEOUT_TRACK 1000000

static void track_device_common(int id, void *arg, char *method);

static void get_devices(struct json_rpc_request *jreq, struct json_object *obj, void *arg)
{
	char *info, *type;
	struct json_object *res = json_array_new(), *dev;
	int i;
	for (i = 0; i < 255; i++) {
		info = driver_get_device_info(i);
		type = driver_get_device_type(i);

		if (type == NULL || info == NULL)
			continue;

		dev = json_object_new();

		json_object_add(dev, "status", json_int_new(driver_get_device_status(i)));
		json_object_add(dev, "type", json_string_new(type));
		json_object_add(dev, "info", json_string_new(info));
		json_object_add(dev, "id", json_int_new(i));

		json_array_add(res, dev);
	}

	json_rpc_return(jreq, res);
}

static void get_device_status(struct json_rpc_request *jreq, struct json_object *obj, void *arg)
{
	if (obj == NULL)
		json_rpc_return(jreq, NULL);

	int status = driver_get_device_status(json_int_get(json_object_get(obj, "id")));

	json_ref_put(obj);

	json_rpc_return(jreq, json_int_new(status));
}

static void set_device_status(struct json_rpc_request *jreq, struct json_object *obj, void *arg)
{
	if (obj == NULL)
		json_rpc_return(jreq, NULL);

	int success = TRUE;

	struct json_object 	*j_id = json_object_get(obj, "id"),
						*j_status = json_object_get(obj, "status");

	if (json_type(j_id) != json_type_int || json_type(j_status) != json_type_int) {
		success = FALSE;
		goto exit;
	}

	int 	id = json_int_get(j_id),
			status = json_int_get(j_status);

	driver_set_device_status(id, status);

exit :
	json_ref_put(obj);
	json_rpc_return(jreq, json_boolean_new(success));
}

static void track_device_common(int id, void *arg, char *method)
{
	struct ws_demo *wsd = (struct ws_demo *)arg;

	struct json_object *req = json_parser_parse("{\"jsonrpc\":\"2.0\", \"id\":null}");
	if (req == NULL) {
		log_warn("Can`t send request : parse_error\n");
		return;
	}

	int rand_num = rand() % MAX_STATUS_VALUE;

	json_object_add(req, "method", json_string_new(method));

	struct json_object *params = json_object_new();
	json_object_add(params, "status", json_int_new(rand_num));
	json_object_add(params, "id", json_int_new(id));

	json_object_add(req, "params", params);

	if (json_rpc_tt_send(wsd->tt, req, NULL, NULL) == -1) {
		log_warn("Can`t send request : tt error\n");
		driver_stop_track(id);
	}

	json_ref_put(req);
}

static void track_device(int id, void *arg)
{
	track_device_common(id, arg, "set_device_status");
}

static void track_rtdevice(int id, void *arg)
{
	track_device_common(id, arg, "set_rtdevice_status");
}

static void device_start_track_common(struct json_rpc_request *jreq, struct json_object *obj, void *arg, void (*cb)(int, void *), int timeout)
{
	if (obj == NULL)
		json_rpc_return(jreq, NULL);

	int success = TRUE;

	struct json_object 	*j_id = json_object_get(obj, "id");

	if (json_type(j_id) != json_type_int) {
		success = FALSE;
		goto exit;
	}

	int id = json_int_get(j_id);

	if (driver_start_track(id, cb, arg, timeout) == -1)
		success = FALSE;

exit :
	json_ref_put(obj);
	json_rpc_return(jreq, json_boolean_new(success));
}

static void device_start_track(struct json_rpc_request *jreq, struct json_object *obj, void *arg)
{
	device_start_track_common(jreq, obj, arg, track_device, DEVICE_TIMEOUT_TRACK);
}

static void rtdevice_start_track(struct json_rpc_request *jreq, struct json_object *obj, void *arg)
{
	device_start_track_common(jreq, obj, arg, track_rtdevice, RTDEVICE_TIMEOUT_TRACK);
}

static void device_stop_track_common(struct json_rpc_request *jreq, struct json_object *obj, void *arg)
{
	if (obj == NULL)
		json_rpc_return(jreq, NULL);

	int success = TRUE;

	struct json_object 	*j_id = json_object_get(obj, "id");

	if (json_type(j_id) != json_type_int) {
		success = FALSE;
		goto exit;
	}

	int id = json_int_get(j_id);

	driver_stop_track(id);

exit :
	json_ref_put(obj);
	json_rpc_return(jreq, json_boolean_new(success));
}

struct json_rpc *init_json_rpc_methods(void *arg)
{
	struct json_rpc *jr = json_rpc_new();

	if (json_rpc_add_method(jr, GET_DEVICES_METHOD, get_devices, arg) == -1)
		goto error;

	if (json_rpc_add_method(jr, GET_DEVICE_STATUS_METHOD, get_device_status, arg) == -1)
		goto error;

	if (json_rpc_add_method(jr, SET_DEVICE_STATUS_METHOD, set_device_status, arg) == -1)
		goto error;

	if (json_rpc_add_method(jr, RTDEVICE_START_TRACK_METHOD, rtdevice_start_track, arg) == -1)
		goto error;

	if (json_rpc_add_method(jr, RTDEVICE_STOP_TRACK_METHOD, device_stop_track_common, arg) == -1)
		goto error;

	if (json_rpc_add_method(jr, DEVICE_START_TRACK_METHOD, device_start_track, arg) == -1)
		goto error;

	if (json_rpc_add_method(jr, DEVICE_STOP_TRACK_METHOD, device_stop_track_common, arg) == -1)
		goto error;

	return jr;

error :
	json_rpc_free(jr);
	return NULL;
}
