/*
 * demo.c
 *
 *  Created on: Mar 14, 2011
 *      Author: ant
 */

#include <json_rpc/json.h>
#include <json_rpc/json_rpc.h>
#include <json_rpc/json_rpc_tt.h>
#include <json_rpc/json_parser.h>

#include <ws/web_sockets.h>

#include <event0/evhttp.h>
#include <event0/evhttps.h>
#include <event.h>

#include "util.h"
#include "driver.h"
#include "log.h"
#include "jrpc_iface.h"
#include "http_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define HTTP_HOST "127.0.0.1"
#define HTTP_PORT 8080

#define SSL_CERT "serv.pem"
#define SSL_KEYFILE "serv.pem"
#define SSL_PASSWD "1234"

#define DEVICES_FILE "devices"

static void ws_errorcb(struct ws_connection *conn, short what, void *arg)
{
	struct ws_demo *wsd = (struct ws_demo *)arg;

	log_info("WebSocket connection closed\n");

	json_rpc_tt_free(wsd->tt);
	json_rpc_free(wsd->jr);

	driver_stop_track_all();

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

	wsd->jr = init_json_rpc_methods(wsd);
	if (wsd->jr == NULL) {
		ws_connection_free(conn);
		free(wsd);
		log_warn("%s : can`t init json_rpc", __func__);
		return;
	}

	wsd->conn = conn;

	wsd->tt = json_rpc_tt_ws_new(wsd->jr, conn, ws_errorcb, wsd);
	if (wsd->tt == NULL) {
		ws_connection_free(conn);
		json_rpc_free(wsd->jr);
		free(wsd);
		log_warn("%s : can`t init ws transport", __func__);
		return;
	}
}

static void sig_handler(int sig, short what, void *arg)
{
	log_warn("Safe Exit\n");
	event_loopexit(NULL);
}

int main (int argc, char **argv)
{
	if (argc < 2) {
		log_error("ERROR : Required parameter - IP ADDRESS - missed!");
		return EXIT_FAILURE;
	}

	event_init();
	if (driver_load_devices(DEVICES_FILE) != 0)
		log_error("ERROR : Can`t load devices\n");

	char *host = argv[1];

	struct evhttp *eh = (argc > 2) ?
						evhttp_start_ssl(host, HTTP_PORT, SSL_CERT, SSL_KEYFILE, SSL_PASSWD) :
						evhttp_start("192.168.0.100", HTTP_PORT);
	if (eh == NULL)
		return EXIT_FAILURE;

	struct ws_accepter *wa = ws_accepter_new(eh, WS_ENDPOINT, ws_acceptcb, NULL);
	if (wa == NULL)
		return EXIT_FAILURE;

	evhttp_set_gencb(eh, http_handler, NULL);

	struct event sig_ev;
	event_set(&sig_ev, SIGINT, EV_SIGNAL, sig_handler, NULL);
	if (event_add(&sig_ev, NULL) == -1)
		return EXIT_FAILURE;

	event_dispatch();

	event_del(&sig_ev);
	ws_accepter_free(wa);
	evhttp_free(eh);
	driver_destroy();

	return 0;
}
