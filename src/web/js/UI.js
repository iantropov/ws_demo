UI = function ()
{
	function initProcessing(canvasId)
	{
		var sketchProc = function(p) {
			p.frameRate(UI.RT_RATE);
			p.size(UI.RT_WIDTH, UI.RT_HEIGHT);

			var rangeY = UI.RT_RANGE;
			var centerY = p.height / 2;
			var scaleY = p.height / rangeY;
			var stepX = p.width / (UI.RT_ARRAY_SIZE - 1);

			p.draw = function() {
				p.background(246, 246, 246);

				p.stroke(25);
				p.strokeWeight(1);
				p.line(0, centerY, p.width, centerY);

				p.stroke(18, 148, 196);
				p.strokeWeight(3);
		
				var lastX = 0, nextX = 0, lastY, nextY;
				var points = $("#" + canvasId).data("points");
				for(var point in points) {
					nextY = p.height - (points[point] * scaleY);
					if(point != 0) {
						p.line(lastX, lastY, nextX, nextY);
						lastX += stepX;
					}
					nextX += stepX;
					lastY = nextY;
				}
			};
		}
		new Processing($("#" + canvasId)[0], sketchProc);
	}

	return {
		RT_RATE: 10,
		RT_HEIGHT: 80,
		RT_WIDTH: 0,
		RT_RANGE: 256,
		RT_ARRAY_SIZE: 50,

		showDevices: function(onButtonClick)
		{
			$("#devices").accordion({ 
					header:			"h3", 
					collapsible: 	true,
					active: 		-1
			});
			
			$("#devices").find(".buttonset").each(function(i, set) {
				$(set).children("input").each(function(idx, elem) {
					$(elem).bind("click", function() {
						var label = $(set).find("label[for='" + elem.id + "']>span");
						label.text((parseInt(label.text()) + 1)% 2);
					
						var value = 0;
						var checks = $(set).find("input");
						var len = checks.length - 1;
						checks.each(function(idx, elem) {
							value += elem.checked * Math.pow(2, len - idx);
						});
					
						onButtonClick(parseInt(set.id.split("cc")[1]), value);
					});
				});
			});
		},
		
		setCurrentStatus: function(device)
		{
			var bits = Util.getBits(device.status);
			if (bits.length > 8)
				return;
			
			var len = bits.length;
			for (var i = 0; i < 8 - len; i++)
				bits.unshift(0);

			$("#cc" + device.id).children("input").each(function(idx, elem) {
				elem.checked = bits[idx];
				$("#cc" + device.id).find("label[for='" + elem.id + "']>span").text(bits[idx]);
			});
			$("#cc" + device.id).buttonset("refresh");
		},
		
		markupDevices: function(devices)
		{
		
			var getTemplate = function(id)
			{
				var templ = [
					'<div class="device">',
						'<h3>',
							'<a href="#">{info:s}</a>',
						'</h3>',
						'<div id="{id:i}" class="devicecontent">',
							'<div style="text-align:center;">'
				];
			
				for (var i = 7; i > -1; i--) {
					templ.push('<div style="padding:0.4em 1em; width:15px;display:inline-block;margin-right:-0.3em;">' + i + '</div>');
				}

				templ.push('<div id="cc{id:i}" class="buttonset">');
				
				for (var i = 0; i < 8; i++) {
					var check = ["check-", id, "-", i].join("");
					templ.push('<input type="checkbox" id="' + check + '" /><label for="' + check + '">0</label>');
				}
				
				templ.concat([
								'</div>',
							'</div>',
						'</div>',
					'</div>'
				]);
				
				return templ;
			}
			
			$.tpl('rtdevice', [
			    '<div class="rtdevice">',
					'<h3>',
						'<a href="#">{info:s}</a>',
					'</h3>',
					'<div id="{id:i}" class="rtdevicecontent">',
						'<div class="start-button" style="margin: 0 40px 10px 0;">Start</div>',
						'<div class="stop-button" style="margin:0 40px 10px 40px;">Stop</div>',
						'<canvas id="canvas{id:i}"></canvas>',
					'</div>',
			    '</div>'
			]);

			this.RT_WIDTH = $("#rtdevices").width() - 80;//if you want be precise, get to mind margins, borders and paddings

			$.each(devices, function(idx, device)
			{
				if (device["type"] == "RT") {
					$.tpl('rtdevice', device).appendTo("#rtdevices");
					initProcessing("canvas" + device.id);
					UI.initRT(device.id);
				} else {
					$.tpl('device' + device.id, getTemplate(device.id));
					$.tpl('device' + device.id, device).appendTo("#devices");
					$("#cc" + device.id).buttonset();
				}
			});		
		},
		
		showRTDevices: function(onStart, onStop)
		{
			$("#rtdevices").accordion({ 
				header:			"h3",
				collapsible: 	true,
				active: 		-1
			});
			
			$(".rtdevicecontent").each(function()
			{
				var device = this;
				var startButton = $(device).children(".start-button");
				var stopButton = $(device).children(".stop-button");
				
				var wrapClick = function(onClick, button)
				{
					return function() {
						if (button.button("option", "disabled"))
							return;
						startButton.button("option", "disabled", !startButton.button("option", "disabled"));
						stopButton.button("option", "disabled", !stopButton.button("option", "disabled"));
						onClick(parseInt(device.id));
					}
				};
				
				startButton.button().bind("click", wrapClick(onStart, startButton));
				stopButton.button().bind("click", wrapClick(onStop, stopButton));
				stopButton.button("option", "disabled", true);
				startButton.button("option", "disabled", false);
			});
		},
		
		refreshRT: function(id, value)
		{
			var points = $("#canvas" + id).data("points");
			
			points.splice(0, 0, value);
			points.pop(); 
		},
		
		initRT: function(id)
		{
			var initialData = [];
			for (var i = 0; i < UI.RT_ARRAY_SIZE; i++)
				initialData.push(0);
			$("#canvas" + id).data("points", initialData);
		}
	}
}();
