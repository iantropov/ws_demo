function JsonRpc()
{
	this.requests = {};
	this.methods = {};
	
	EBus.declare("jrpc_request", "jrpc_response");
	
	return this;
}

JsonRpc.prototype.processRequest = function(req)
{
	var method = req.method;
	if (!Util.isDefined(method) || !Util.isDefined(this.methods[method]))
		return;
	
	EBus.fire("jrpc_req_in", req);
		
	var result = this.methods[method](req.params);
	if (Util.isDefined(req.id))
		return;
		
	return {	jsonrpc : "2.0",
				result : result,
				id : req.id };
}

JsonRpc.prototype.processResponse = function(resp)
{
	if (!Util.isDefined(resp.id) || resp.id === null || !Util.isDefined(this.requests[resp.id]))
		return;
		
	EBus.fire("jrpc_resp_in", resp);
		
	this.requests[resp.id](Util.isDefined(resp.result) ? resp.result : resp.error);	
}

JsonRpc.prototype.preprocessRequest = function(req, callback, scope)
{
	EBus.fire("jrpc_req_out", req);
	
	if (Util.isDefined(req.id) && Util.isDefined(callback))
		this.requests[req.id] = Util.createDelegate(callback, scope);
}

JsonRpc.prototype.addMethod = function(name, method, scope)
{
	this.methods[name] = Util.createDelegate(method, scope);
}

JsonRpc.prototype.removeMethod = function(name)
{
	delete this.methods[name];
}

JsonRpcWebSocket = function(jrpc, ws)
{
	this.jrpc = jrpc;
	this.ws = ws;
	
	ws.onmessage = Util.createDelegate(this.process, this);
}

JsonRpcWebSocket.prototype.process = function(message)
{
	var obj = Util.decodeJSON(message.data);
	
	var res;
	if (Util.isDefined(obj.params))
		res = this.jrpc.processRequest(obj);
	else
		this.jrpc.processResponse(obj);
	
	if (Util.isDefined(res))
		this.ws.send(Util.encodeJSON(res));
}

JsonRpcWebSocket.prototype.sendRequest = function(req, callback, scope)
{
	this.jrpc.preprocessRequest(req, callback, scope);
	
	this.ws.send(Util.encodeJSON(req));
}

JsonRpcHttp = function(jrpc, uri)
{
	this.jrpc = jrpc;
	this.uri = uri;
}

JsonRpcHttp.prototype.sendRequest = function(req, callback, scope)
{
	this.jrpc.preprocessRequest(req, callback, scope);
		
	$.ajax({
		url:		this.uri,
		type:		"POST",
		success: 	Util.createDelegate(this.jrpc.processResponse, this.jrpc),
		data:		Util.encodeJSON(req),
		dataType:	"json",
		contentType:	"application/json-rpc"
	});
}

