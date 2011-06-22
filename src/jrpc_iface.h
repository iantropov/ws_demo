/*
 * jrpc_iface.h
 *
 *  Created on: Jun 20, 2011
 *      Author: ant
 */

#ifndef JRPC_IFACE_H_
#define JRPC_IFACE_H_

#include <json_rpc/json_rpc.h>

struct ws_demo {
	struct json_rpc *jr;
	struct json_rpc_tt *tt;

	struct ws_connection *conn;
};

struct json_rpc *init_json_rpc_methods(void *cb_arg);

#endif /* JRPC_IFACE_H_ */
