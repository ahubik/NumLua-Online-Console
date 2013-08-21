/*This software is provided under the MIT Licence (http://opensource.org/licenses/MIT):

The MIT License (MIT)

Copyright (c) 2013 Luis Carvalho and Alexander Hubik

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */
Settings = { //Some variable likely to be unique to one instance of use, currently only server info
		ServerAddr : "ws://localhost:6582",
		Protocol : "repl"
};


$(document).ready(function() {
	/* Define jquery fns here */
	//Make places for output functions to go later when console is initialized
	var DisplayFunction = {};
	var ImageDisplayFunction = {};
	
	MakeConsole(); //Does what it says
	
	$("#btnConnect").click(function() {
		Connect()
	});
	
	//Test button is currently disabled.  Can be enabled from index.html
	$("#btnTest").click(function() {
		Serverws.send('print(\"Hello, World.\")');
	});
});

//This doesn't seem to work
$(".statusfield").change(function() {
	console.log('scrolling to bottom');
	scrollToBottom();
});

function Connect() {
	//get address and protocol from settings at top of page
	address = Settings.ServerAddr;
	protocol = Settings.Protocol;
	
	//Construct an alert to inform user of connection attempt
	var alerttext = "\nConnecting to server at ";
	alerttext = alerttext.concat(address);
	alerttext = alerttext.concat(" with protocol ");
	alerttext = alerttext.concat(protocol);
	
	//Display alert and open and set up WebSocket connection
	$("#statusfield").append(alerttext);
	Serverws = new WebSocket(address, protocol);
	Serverws.onmessage = function(evt) {
		onMessage(evt);
	}
};

function MakeConsole() {
	//Set up a <div> tag for the console to live in
	var consoleDiv = $('<div id="ConsoleDiv" class="console">');
	$('#ConsoleContainer').append(consoleDiv);
	$('#ConsoleDiv').stop().animate({ //This doesn't work either.  All scrolling is manual.
		  scrollTop: $("#ConsoleDiv")[0].scrollHeight
		}, 800);
	
	//The actual console object.
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
			if (line == "image") //This is a very crude system for implementing commands that are directed to the client, not the server.
				//ImageDisplayFunction($("<img src=\"test-image.jpg\" alt=\"A test image should be here.\" height=\"100\" width=\"100\">"));
				console.log("Image test called")
			else
				Serverws.send(line);
			return true;

		},
		promptHistory : true,
		continuedPrompt : false
		//autofocus : true //Reinstate if desired
	});
	//Put output functions somewhere where they can be accessed from code not in MakeConsole
	DisplayFunction = controller.display;
	ImageDisplayFunction = controller.displayImage;
	
}



function onMessage(evt) { //Handle received messages
	//Is it an error message?
	err = Boolean(evt.data.charCodeAt(0));
	
	//Get just the is text
	mesg = evt.data.slice(1);
	
	//Make it the appropriate color for a message of it's kind
	color = 'blue';
	if (err)
		color = 'red';
	
	DisplayFunction(mesg, color);
};