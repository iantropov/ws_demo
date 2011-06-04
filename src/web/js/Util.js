Util = (function()
{
	var id = 0;
	return {
		getId:function()
		{
			return id++;
		},
		
		isDefined: function(val)
		{
			return (val !== undefined);
		},
		
		encodeJSON: function(val)
		{
			return JSON.stringify(val);
		},
		
		decodeJSON: function(json)
		{
			return JSON.parse(json);
		},
		
		createDelegate: function(func, scope)
		{
			var wrap = function()
			{
				func.apply(scope, arguments);
			};
			
			return wrap;			
		},
		
		jsonRpcRequest: function(method, params)
		{	
			var req = {"jsonrpc":"2.0", "method":method, "id":Util.getId()};
			if (params)
				req["params"] = params;
				
			return req;
		},
		
		createWSURI: function(location, uri)
		{
			var scheme = (location.protocol == "https:") ? "wss" : "ws";
			var host = location.host;
			
			return $.format("{0}://{1}{2}", scheme, host, uri);
		}
	};
})();
