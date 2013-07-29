/**
 * Copyright (c) 2013 Luis Carvalho and Alexander Hubik
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy
 *of this software and associated documentation files (the "Software"), to deal
 *in the Software without restriction, including without limitation the rights
 *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *copies of the Software, and to permit persons to whom the Software is
 *furnished to do so, subject to the following conditions:
 *
 *The above copyright notice and this permission notice shall be included in
 *all copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *THE SOFTWARE.
 */

$(document).ready(function() {
	/**
	 * Things that happen as soon as open, most of everything. Define jquery fns
	 * here. *
	 */
	var DisplayFunction = {};
	MakeConsole();
	$("#btnConnect").click(function() {
		Connect()
	});

	$("#btnTest").click(function() {
		Serverws.send('print(\"Hello, World.\")');
	});

	$('#input').keypress(function(event) {
		// console.log('Jquery Handler for input.keypress() called.');
		which = String(event.which);
		// console.log(which);
		if (event.which == 13) {
			console.log('sending');
			texttosend = $("#input").val();
			console.log(texttosend);
			sendnewtext();
		}
	});
});

$(".statusfield").change(function() {
	console.log('scrolling to bottom');
	scrollToBottom();
});

function Connect() {
	//address = new String($("#addressfield").val());uncomment this if server address needs to be set every time
	address = "ws://localhost:6582"; // change this to change server address for long-term change in server location
	protocolexists = new Boolean(false);

	/*if ($("#protocolfield").val() != "") {
		protocolexists = true;
	}*/
	
	var alerttext = "\nConnecting to server at ";
	alerttext = alerttext.concat(address);
	if (protocolexists) {
		protocol = new String($("#protocolfield").val()); 
		alerttext = alerttext.concat(" with protocol ");
		alerttext = alerttext.concat(protocol);
	}
	
	protocolexists = true; // <--Kludge
	
	$("#statusfield").append(alerttext);
	if (protocolexists) {
		Serverws = new WebSocket(address, "repl"); // Fix this again, but
		// only if there's a
		// problem
	} else {
		Serverws = new WebSocket(address);
	}
	Serverws.onmessage = function(evt) {
		// console.log('Received a message.');
		onMessage(evt);
	}
};

function MakeConsole() {
	var consoleDiv = $('<div class="console">');
	$('#ConsoleContainer').append(consoleDiv);

	var controller = consoleDiv.console({
		promptLabel : '>> ',
		continuedPromptLabel : ' -> ',
		commandValidate : function(line) {
			if (line == "")
				return false;
			else
				return true;
		},
		commandHandle : function(line, report) {

			controller.continuedPrompt = false;
			//console.log("Execute: " + line);
			//report("sending " + line);
			Serverws.send(line);
			return true;

		},
		promptHistory : true,
		continuedPrompt : false
		//autofocus : true
	});
	//controller.display("The function extern.notice has been called")
	function Display(text) {
		controller.display(text);
	}
	DisplayFunction = controller.display;
}



function onMessage(evt) {
	console.log("got message");
	DisplayFunction(evt.data);
	// $("#statusfield").append("\nRecived message.");
};
