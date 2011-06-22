/*
 * http_handler.c
 *
 *  Created on: Jun 20, 2011
 *      Author: ant
 */

#include "http_handler.h"
#include "util.h"

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

void http_handler(struct evhttp_request *req, void *arg)
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
