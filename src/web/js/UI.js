UI = function ()
{
	return {
		showDevices: function(onExpand, onButtonClick)
		{
			var currentId;
			var onExpandWrap = function(event, ui)
			{
				if (ui.newContent.length == 0)
					onExpand(true, parseInt(currentId));
				else
					onExpand(false, parseInt(currentId = ui.newContent[0].id));
			};
			
			$("#devices").accordion({ 
					header:			"h3", 
					collapsible: 	true,
					active: 		-1,
					change: 		onExpandWrap
			});
			$(".devicecontent").each(function()
			{
					var device = this;
					$(device).children(".button").button().bind("click", function(){
						var status = parseInt($(device).children("input")[0].value);
						onButtonClick(parseInt(device.id), status);
					});
			});

		},
		
		setCurrentStatus: function(device)
		{
			$("#" + device.id + " .status").html(device.status);
		},
		
		markupDevices: function(devices)
		{
			$.tpl('device', [
			    '<div class="device">',
				'<h3>',
					'<a href="#">{info:s}</a>',
				'</h3>',
				'<div id="{id:i}" class="devicecontent">',
					'<div class="output">',
						'<span>Current status : </span>',
						'<span class="status">{status:i}</span>',
					'</div>',
					'<input class="input" type="text"></input>',
					'<div class="button" style="margin-left:40px">Send</div>',
				'</div>',
			    '</div>'
			]);
			
			$.each(devices, function(idx, device)
			{
					$.tpl('device', device).appendTo("#devices");
			});
		}
	}
}();
