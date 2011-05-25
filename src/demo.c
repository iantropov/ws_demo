/*
 * demo.c
 *
 *  Created on: Mar 14, 2011
 *      Author: ant
 */

#include <json_rpc/json.h>
#include <json_rpc/json_rpc.h>
#include <event0/evhttp.h>
#include <event0/evhttps.h>
#include <event.h>

#include "util.h"
#include "driver.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HTTP_HOST "127.0.0.1"
#define HTTP_PORT 8080

#define SSL_CERT "serv.pem"
#define SSL_KEYFILE "serv.pem"
#define SSL_PASSWD "1234"

#define GET_DEVICES_METHOD "get_devices"
#define GET_DEVICE_STATUS_METHOD "get_device_status"
#define SET_DEVICE_STATUS_METHOD "set_device_status"

#define JSON_RPC_ENDPOINT "/devices"

#define HTTP_OK_CODE 200
#define HTTP_NOT_FOUND_CODE 404
#define HTTP_INTERNAL_ERROR_CODE 500
#define HTTP_OK_REASON "OK"
#define HTTP_NOT_FOUND_REASON "Not Found"
#define HTTP_INTERNAL_ERROR_REASON "Internal Error"

#define CSS_POSTFIX ".css"

#define DEVICES_FILE "devices"

void get_devices(struct json_rpc_request *jreq, struct json_object *obj, void *arg)
{
	char *info;
	struct json_object *res = json_array_new(), *dev;
	int i;
	for (i = 0; i < 255; i++) {
		info = driver_get_device_info(i);
		if (info != NULL) {
			dev = json_object_new();
			json_object_add(dev, "info", json_string_new(info));
			json_object_add(dev, "status", json_int_new(driver_get_device_status(i)));
			json_object_add(dev, "id", json_int_new(i));
			json_array_add(res, dev);
		}
	}

	json_rpc_return(jreq, res);
}

void get_device_status(struct json_rpc_request *jreq, struct json_object *obj, void *arg)
{
	if (obj == NULL)
		json_rpc_return(jreq, NULL);

	int status = driver_get_device_status(json_int_get(json_object_get(obj, "id")));

	json_ref_put(obj);

	json_rpc_return(jreq, json_int_new(status));
}

void set_device_status(struct json_rpc_request *jreq, struct json_object *obj, void *arg)
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

void resource_handler(struct evhttp_request *req, void *arg)
{
	if (strcmp(req->uri, JSON_RPC_ENDPOINT) == 0)
		return;

	char *path = req->uri + sizeof(char);//skip leading slash

	int len = get_file_size(path);
	if (len == -1) {
		evhttp_send_error(req, HTTP_NOT_FOUND_CODE, HTTP_NOT_FOUND_REASON);
		return;
	}

	void *buf = get_file_content(path);
	if (buf == NULL)
		goto error;

	struct evbuffer *evbuf = evbuffer_new();
	if (evbuf == NULL)
		goto error;

	if (evbuffer_add(evbuf, buf, len) == -1) {
		evbuffer_free(evbuf);
		goto error;
	}

	if (string_ends_by(path, CSS_POSTFIX)) {
		evhttp_add_header(req->output_headers, "Content-Type", "text/css");
	}

	evhttp_send_reply(req, HTTP_OK_CODE, HTTP_OK_REASON, evbuf);

	free(buf);
	free(evbuf);

	return;

error:
	free(buf);
	evhttp_send_error(req, HTTP_INTERNAL_ERROR_CODE, HTTP_INTERNAL_ERROR_REASON);
}

struct json_rpc *init_json_rpc_methods()
{
	struct json_rpc *jr = json_rpc_new();

	if (json_rpc_add_method(jr, GET_DEVICES_METHOD, get_devices, NULL) == -1)
		goto error;

	if (json_rpc_add_method(jr, GET_DEVICE_STATUS_METHOD, get_device_status, NULL) == -1)
		goto error;

	if (json_rpc_add_method(jr, SET_DEVICE_STATUS_METHOD, set_device_status, NULL) == -1)
		goto error;

	return jr;

error :
	json_rpc_free(jr);
	return NULL;
}

int main ()
{
	event_init();
	if (driver_init(DEVICES_FILE) != 0)
		log_error("ERROR : Can`t init driver\n");

	struct evhttp *eh = evhttp_start_ssl(HTTP_HOST, HTTP_PORT, SSL_CERT, SSL_KEYFILE, SSL_PASSWD);
	if (eh == NULL)
		return EXIT_FAILURE;

	struct json_rpc *jr = init_json_rpc_methods();
	if (jr == NULL)
		return EXIT_FAILURE;

	evhttp_set_json_rpc(eh, JSON_RPC_ENDPOINT, jr);
	evhttp_set_gencb(eh, resource_handler, NULL);

	event_dispatch();

	evhttp_free(eh);
	json_rpc_free(jr);
	driver_destroy();

	return 0;
}
