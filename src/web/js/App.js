
function initWebSocketConnection()
{
	var ws = new WebSocket(Util.createWSURI(window.location, "/ws"));
	ws.onerror = function(){alert("Error in WebSocket connection!");};
	ws.onclose = function(){alert("WebSocket connection closed!");};
	
	return ws;
}

function initJrpcWebSocket(webSocket)
{
	var jrpc = new JsonRpc();
	jrpc.addMethod("set_device_status", UI.setCurrentStatus);
	jrpc.addMethod("set_rtdevice_status", function(params){UI.refreshRT(params.id, params.status)});
	
	var jrpc_ws = new JsonRpcWebSocket(jrpc, webSocket);
	
	return jrpc_ws;
}

function initConsole(jrpc_ws)
{
	$("#send_request").button().bind("click", function() {
		var method = $("#method_name").val();
		if (method == "") {
			alert("Enter method name!");
			return;
		}
		
		var params_raw = $("#method_params").val();
		var params_decoded = Util.decodeJSON(params_raw);
		if (params_raw &&  params_decoded == null) {
			alert("Enter correct params!");
			return;
		}
		var params = params_raw ? params_decoded : "";
		
		if ($("#is_notify").attr("checked")) {
			jrpc_ws.sendRequest(Util.jsonRpcRequest(method, params));
		} else {
			jrpc_ws.sendRequest(Util.jsonRpcRequest(method, params), function(response) {
				alert(Util.encodeJSON(response));
			});
		}
	});
	
	$("#is_notify").button();
}

function initDevices(jrpc_ws, webSocket)
{		
	
	EBus.listen("ui_rtdevice_start_track", function(id) {
		jrpc_ws.sendRequest(
			Util.jsonRpcRequest('rtdevice_start_track', {id: id})
		);
	});
	
	EBus.listen("ui_rtdevice_stop_track", function(id) {
		jrpc_ws.sendRequest(
			Util.jsonRpcRequest('rtdevice_stop_track', {id: id})
		);
	});
	
	EBus.listen("ui_device_change_status", function(id, status) {
		jrpc_ws.sendRequest(
			Util.jsonRpcRequest('set_device_status', {id: id, status: status})
		);
	});
	
	webSocket.onopen = function()
	{
		jrpc_ws.sendRequest(
			Util.jsonRpcRequest('get_devices'), 
			function(devices) {
				UI.markupDevices(devices);
				UI.showDevices();
				UI.showRTDevices();
		
				$(devices).each(function(idx, device) {
					if (idx % 2 != 0 && device.type == "byte")
						jrpc_ws.sendRequest(Util.jsonRpcRequest('device_start_track', {id: device.id}));
				});
			}
		);
	}
};

function initStatsPanel()
{
	var t = null;
	
	function turn_off() {
		$("#light").removeClass("light-on");
		t = null;
	};

	function wrapStats(stat) {
		return function() {
			$("#info").data(stat, $("#info").data(stat) + 1);
			
			UI.setStats($("#info").data("reqs"), $("#info").data("resps"));
		
			if (t != null)
				return;
			
			$("#light").addClass("light-on");
			t = setTimeout(turn_off, 100);
		};
	};
	
	$("#info").data("reqs", 0);
	$("#info").data("resps", 0);
				
	EBus.listen("jrpc_req_in", wrapStats("reqs"));
	EBus.listen("jrpc_req_out", wrapStats("resps"));
};
		
function appEntryPoint()
{
	var webSocket = initWebSocketConnection();
	var jrpc_ws = initJrpcWebSocket(webSocket);

	initDevices(jrpc_ws, webSocket);
	initConsole(jrpc_ws);
	
	initStatsPanel();
}

