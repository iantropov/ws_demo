EBus = (function()
{
	var events = {};
	return {
		fire: function(event)
		{
			var args = $.makeArray(arguments).slice(1);
			if (!Util.isDefined(events[event]))
				return;
				
			$.each(events[event], function(idx, handler) {
				handler.fn.apply(handler.scope, args);
			});
		},
		listen: function(event, handler, scope)
		{
			events[event] = events[event] || [];
			events[event].push({
				fn: handler,
				scope: scope
			});
		},
		declare: function(events)
		{
			$.each(events, function(idx, event) {
				events[event] = events[event] || [];
			});
		}
	};
})();
