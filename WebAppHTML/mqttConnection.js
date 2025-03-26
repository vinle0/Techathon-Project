/*
   Eclipse Paho MQTT-JS Utility
   by Elliot Williams for Hackaday article

   Hacked up by:  Mark McDermott for EE445L Lab 4E
   On:            5/29/23   
*/

// ---------------------------------------
// Global variables
//

var client      = null;
// var hour        = "";
// var minute      = "";
// var second      = "";
// var ampm        = "";
// var mil_time    = "";
var page_num = 1;


// TODO: update eid with your own.
var hostname        = "10.159.177.113";
var port            = "9001";
var eid             = "msc3785"
var clientId        = "mqtt_ee445l_" + eid;

// var hostname        = "10.159.177.60";
// var port            = "1883";
// var eid             = "msc3785"
// var clientId        = "mqtt_ee445l_" + eid;

// Subscribe (from board_2_webpage)

var test 	= 	eid + "/test";
// var hour_bd =   eid + "/b2w/hour"; 
// var min_bd  =   eid + "/b2w/min";
// var sec_bd  =   eid + "/b2w/sec"; 
// var mode_bd =   eid + "/b2w/mode";
// var sensor_bd = eid + "/b2w/sensor"; 
var cmd_bd = 	eid + "/b2w/cmd";
// Publish (from webpage_2_board) 
 
var w2b   =  eid + "/w2b";

// -----------------------------------------------------------------------
// This is called after the webpage is completely loaded
// It is the main entry point into the JS code

function connect() {
	// Set up the client
	// TODO: We use a default here for emqx, but if you're using ECE445L broker,
	// feel free to replace with the hostname + port specified earlier. 
	//const url = 'ws://broker.emqx.io:8083/mqtt';
	const url = 'ws://10.159.177.60:9001/mqtt';
	const options = {
		// Clean session
		clean: true,
		connectTimeout: 4000,
		// Authentication
		clientId: eid,
		username: null,
		password: null,
	};
	client  = mqtt.connect(url, options);
	client.on('connect', function () {
		onConnect();
	});

	// Receive messages
	client.on('message', function (topic, payload, packet) {
	  	onMessageArrived(topic, payload);
	});
}

function onConnect() {
	console.log("Client Connected.");
    
	// TODO: subscribe to your desired topics.
	client.subscribe(test);
	// client.subscribe(hour_bd);
	// client.subscribe(min_bd);
	// client.subscribe(sec_bd);
    // client.subscribe(mode_bd);
	// client.subscribe(sensor_bd);
	client.subscribe(cmd_bd);
}

function onMessageArrived(topic, message) {
	console.log(topic, String(message));

	// TODO: call method to update logic and update UI.
	// Update element depending on which topic's data came in.
	switch (topic) {
		case test: {
			console.log("Test message!");
			break;
		}
		case cmd_bd: {
			// TODO DO SOMETHING HERE
			switch (String(message).toLowerCase()) {
				case "prev", "0": {
					console.log("Turning Prev Page...");
					turnPrevPage();
					break;
				}
				case "save", "1": {
					console.log("Saving Note...");
					saveNote();
					break;
				}
				case "next", "2": {
					console.log("Turning Next Page...");
					turnNextPage();
					break;
				}
				default: {
					console.log("COMMAND NOT RECOGNIZED! CMD: " + String(message));
					console.log("Message Length: " + String(message).length);
				}
			}
			console.log("CMD RECEIVED: " + message);
			break;
		}
		default:
			break;
	} 
}

// SAMPLE FUNCTION TO PUBLISH TO W2B
// -----------------------------------------------------------------------
// Provides the button logic that toggles the mode
// Triggered by pressing the HTML button "12/24"
// 
// function toggleMode() {
// 	var payload = "7";
// 	console.log("Publishing ", payload, " to ", w2b);
// 	client.publish(w2b, payload);
// }

// //////////////////////////////////////////////////////////////////////////
//
//  ADD MORE FUNCTIONS HERE
//
// //////////////////////////////////////////////////////////////////////////
function turnNextPage() {
	page_num++;
	Update_Page_Number();
	client.publish(w2b, "0");
	console.log("Next Page Turned");
}
function turnPrevPage() {
	page_num--;
	Update_Page_Number();
	client.publish(w2b, "1");
	console.log("Prev Page Turned");
}
function saveNote() {
	// DO SOMETHING
	console.log("Note Saved");
}


// -----------------------------------------------------------------------
// This function appends AM or PM to the time when not in 24 hour mode
//
function Update_Page_Number() {
  
    // Adding time elements to the div
    document.getElementById("details-subheading").innerText = "(page " + page_num + ")";
  
    // Set Timer to 1 sec (1000 ms)
    setTimeout(Update_Page_Number, 1000);
}

Update_Page_Number();
