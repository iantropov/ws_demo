/*
 * demo.c
 *
 *  Created on: Mar 14, 2011
 *      Author: ant
 */

#include <json_rpc/json.h>
#include <json_rpc/json_rpc.h>
#include <json_rpc/json_rpc_tt.h>

#include <ws/web_sockets.h>

#include <event0/evhttp.h>
#include <event0/evhttps.h>
#include <event.h>

#include "util.h"
#include "driver.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define HTTP_HOST "127.0.0.1"
#define HTTP_PORT 8080

#define SSL_CERT "serv.pem"
#define SSL_KEYFILE "serv.pem"
#define SSL_PASSWD "1234"

#define GET_DEVICES_METHOD "get_devices"
#define GET_DEVICE_STATUS_METHOD "get_device_status"
#define SET_DEVICE_STATUS_METHOD "set_device_status"

#define JSON_RPC_ENDPOINT "/devices"
#define WS_ENDPOINT "/ws"

#define PATH_PREFIX "web"

#define ROOT_URI "/index.html"

#define HTTP_OK_CODE 200
#define HTTP_NOT_FOUND_CODE 404
#define HTTP_INTERNAL_ERROR_CODE 500
#define HTTP_OK_REASON "OK"
#define HTTP_NOT_FOUND_REASON "Not Found"
#define HTTP_INTERNAL_ERROR_REASON "Internal Error"

#define CSS_POSTFIX ".css"
#define JS_POSTFIX ".js"
#define PNG_POSTFIX ".png"

#define DEVICES_FILE "devices"

struct ws_demo {
	struct json_rpc_tt *tt;
	struct event ev;

	int counter;
};

static struct json_rpc *__jr;

static void get_devices(struct json_rpc_request *jreq, struct json_object *obj, void *arg)
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

static char *get_path(char *uri)
{
	int len = strlen(uri) + strlen(PATH_PREFIX) + 1;
	char *p = calloc(len, sizeof(char));
	if (p == NULL)
		return NULL;

	strcat(p, PATH_PREFIX);
	strcat(p, uri);

	return p;
}

static void resource_handler(struct evhttp_request *req, void *arg)
{
	if (strcmp(req->uri, JSON_RPC_ENDPOINT) == 0 || strcmp(req->uri, WS_ENDPOINT) == 0)
		return;

	char *path = (strcmp(req->uri, "/") == 0) ? get_path(ROOT_URI) : get_path(req->uri);
	if (path == NULL)
		goto error;

	int len = get_file_size(path);
	if (len == -1) {
		evhttp_send_error(req, HTTP_NOT_FOUND_CODE, HTTP_NOT_FOUND_REASON);
		free(path);
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

	if (string_ends_by(path, JS_POSTFIX)) {
		evhttp_add_header(req->output_headers, "Content-Type", "text/javascript");
	}

	if (string_ends_by(path, PNG_POSTFIX)) {
		evhttp_add_header(req->output_headers, "Content-Type", "image/png");
	}

	evhttp_send_reply(req, HTTP_OK_CODE, HTTP_OK_REASON, evbuf);

	free(path);
	free(buf);
	free(evbuf);

	return;

error:
	free(path);
	free(buf);
	evhttp_send_error(req, HTTP_INTERNAL_ERROR_CODE, HTTP_INTERNAL_ERROR_REASON);
}

static struct json_rpc *init_json_rpc_methods()
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

static void ws_send_request(int fd, short what, void *arg)
{
	struct ws_demo *wsd = (struct ws_demo *)arg;

//	if (wsd->counter++ == 3) {
//		json_rpc_tt_free(wsd->tt);
//		free(wsd);
//		return;
//	}

	struct json_object *req = json_object_new();

	struct json_object *params = json_object_new();

	int st = driver_get_device_status(1);

	if (st++ >= 255)
		st = 0;

	driver_set_device_status(1, st);

	json_object_add(params, "status", json_int_new(st));
	json_object_add(params, "id", json_int_new(1));

	json_object_add(req, "jsonrpc", json_string_new("2.0"));
	json_object_add(req, "method", json_string_new("set_device_status"));
	json_object_add(req, "params", params);
	json_object_add(req, "id", json_null_new());

	if (json_rpc_tt_send(wsd->tt, req, NULL, NULL) == -1)
		log_warn("%s : json_rpc_tt_send failed", __func__);

	json_ref_put(req);

	struct timeval tv = {7, 0};
	if (event_add(&wsd->ev, &tv) == -1) {
		json_rpc_tt_free(wsd->tt);
		free(wsd);
		log_warn("%s : event_add failed", __func__);
	}
}

static void ws_errorcb(struct ws_connection *conn, short what, void *arg)
{
	struct ws_demo *wsd = (struct ws_demo *)arg;

	log_info("Some error in ws");

	json_rpc_tt_free(wsd->tt);
	event_del(&wsd->ev);

	free(wsd);
}

static void ws_acceptcb(struct ws_accepter *wa, struct bufevent *bufev, void *arg)
{
	struct ws_connection *conn = ws_connection_new(bufev, NULL, NULL, NULL);
	if (conn == NULL) {
		log_warn("%s : can`t establish web socket connection", __func__);
		return;
	}

	struct ws_demo *wsd = (struct ws_demo *)calloc(1, sizeof(struct ws_demo));
	if (wsd == NULL) {
		ws_connection_free(conn);
		log_warn("%s : calloc failed", __func__);
		return;
	}

	wsd->tt = json_rpc_tt_ws_new(__jr, conn, ws_errorcb, wsd);
	if (wsd->tt == NULL) {
		ws_connection_free(conn);
		free(wsd);
		log_warn("%s : can`t init ws transport", __func__);
		return;
	}

	struct timeval tv = {7, 0};
	event_set(&wsd->ev, -1, EV_TIMEOUT, ws_send_request, wsd);
	if (event_add(&wsd->ev, &tv) == -1) {
		ws_connection_free(conn);
		free(wsd);
		log_warn("%s : event_add failed", __func__);
	}
}

static void sig_handler(int sig, short what, void *arg)
{
	log_warn("Safe Exit\n");
	event_loopexit(NULL);
}

int main (int argc, char **argv)
{
	event_init();
	if (driver_init(DEVICES_FILE) != 0)
		log_error("ERROR : Can`t init driver\n");

	char *host = (argc > 1) ? argv[1] : "192.168.0.100";

	struct evhttp *eh = (argc > 2) ?
						evhttp_start_ssl(host, HTTP_PORT, SSL_CERT, SSL_KEYFILE, SSL_PASSWD) :
						evhttp_start(host, HTTP_PORT);
	if (eh == NULL)
		return EXIT_FAILURE;

	__jr = init_json_rpc_methods();
	if (__jr == NULL)
		return EXIT_FAILURE;

	struct json_rpc_tt *tt = json_rpc_tt_http_new(__jr, eh, JSON_RPC_ENDPOINT);
	if (tt == NULL)
		return EXIT_FAILURE;

	struct ws_accepter *wa = ws_accepter_new(eh, WS_ENDPOINT, ws_acceptcb, NULL);
	if (wa == NULL)
		return EXIT_FAILURE;

	evhttp_set_gencb(eh, resource_handler, NULL);

	struct event sig_ev;
	event_set(&sig_ev, SIGINT, EV_SIGNAL, sig_handler, NULL);
	if (event_add(&sig_ev, NULL) == -1)
		return EXIT_FAILURE;

	event_dispatch();

	event_del(&sig_ev);
	json_rpc_tt_free(tt);
	ws_accepter_free(wa);
	evhttp_free(eh);
	json_rpc_free(__jr);
	driver_destroy();

	return 0;
}
