/*
 * http_handler.h
 *
 *  Created on: Jun 20, 2011
 *      Author: ant
 */

#ifndef HTTP_HANDLER_H_
#define HTTP_HANDLER_H_

#include <event0/evhttp.h>

#define JSON_RPC_ENDPOINT "/devices"
#define WS_ENDPOINT "/ws"

void http_handler(struct evhttp_request *req, void *arg);


#endif /* HTTP_HANDLER_H_ */
