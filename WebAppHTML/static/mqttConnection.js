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

// ---------------- NOTES VARIABLES --------------------------------------
var notesList = [];
var notesPageNumberList = [];
var numNotes = 0;
var currentNoteIndex = 1;



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
	FH_TestFunction();
}
function turnPrevPage() {
	page_num--;
	Update_Page_Number();
	client.publish(w2b, "1");
	console.log("Prev Page Turned");
}

var TEXT_CUTOFF = 20;
var retry = 0;
function saveNote() {
	retry = 0;
	saveNoteHelper(0)
}
function saveNoteHelper(retryNum) {
	if (retryNum > 3) {
		return;
	}
	try {
		// DO SOMETHING
		var text = document.getElementById("noteTextArea").value;
		if (currentNoteIndex > numNotes) {
			notesList.push(text);
			notesPageNumberList.push(page_num);
			// Create Note Entry
			var noteEntryButton = document.createElement("BUTTON");
			noteEntryButton.id = currentNoteIndex.toString();
			console.log(text.length);
			noteEntryButton.textContent = "Page " + notesPageNumberList[currentNoteIndex - 1].toString() + ":" + text.length > TEXT_CUTOFF? text.substring(0,TEXT_CUTOFF - 3) + "..." : text;
			function onAction() {
				var textArea = document.getElementById("noteTextArea");
				currentNoteIndex = parseInt(noteEntryButton.id);
				textArea.value = notesList[currentNoteIndex - 1];
			}
			noteEntryButton.addEventListener('click', onAction);
			noteEntryButton.style = "background-color:rgb(176, 218, 212); border-color: black; width: 600px; height: 40px";
			document.getElementById("notesParagraph").appendChild(noteEntryButton);
			if (numNotes == 0) {
				document.getElementById("noNotes").remove();
			}
			if (currentNoteIndex - 1 == numNotes) {
				numNotes++
			} else {
				console.log("SOMETHING WRONG HAPPENED, NumNotes=" + numNotes);
				numNotes = notesList.length;
			}
		}
		else {
			notesList[currentNoteIndex - 1] = text;
			document.getElementById(currentNoteIndex.toString()).textContent = text.length > 20? text.substring(0,20) + "..." : text;
			console.log(text.length);

		}
		console.log("Note Saved");
	} catch (error) {
		console.log(error);
		retry++;
		console.log("Retrying! " + retry);
		saveNoteHelper(retry);
	}

}
function newNote() {
	currentNoteIndex = numNotes + 1;
	document.getElementById("noteTextArea").value = "";
}


// -----------------------------------------------------------------------
// This functions update visual elements of the website
//
function Update_Page_Number() {
  
    // Adding time elements to the div
    document.getElementById("details-subheading").innerText = "(page " + page_num + ")";
  
    // Set Timer to 1 sec (1000 ms)
    setTimeout(Update_Page_Number, 1000);
}
Update_Page_Number();
// ------------------ GENERATIVE AI STUFF ---------------------------------
const message_header_old = "Here are some notes a user took while reading a book. First, determine whether the notes are from a fictional or nonfictional book. Then, based on that classification, generate appropriate study materials. For fiction, provide a summary, key themes, and 3 discussion questions. For nonfiction, provide a summary, key concepts, and 3 quiz questions.";
const message_header_old2 = "A user may provide notes from a book they're reading, or they may not provide any notes at all.\n\nIf notes are provided, first determine whether the book is fiction or nonfiction based on the content of the notes.\n\nThen generate appropriate study materials:\n\nFor fiction, include:\n• a brief summary\n• key characters\n• main themes or motifs\n• 3 discussion or essay questions\n\nFor nonfiction, include:\n• a summary of key concepts\n• definitions or explanations of important terms\n• 3 quiz questions (multiple choice or short answer)\n\nIf no notes are provided, respond by asking the user to share some notes, or alternatively ask what book they're reading so you can help based on that."
const message_header = "A user may or may not provide notes from a book they are reading. Your task is to:\n\nIf notes are provided:\na. Try to determine whether the notes are from a fiction or nonfiction book.\nb. If they are clearly book-related, generate appropriate study materials:\n- For fiction:\n• A brief plot summary\n• Key characters\n• Main themes or symbols\n• 3 discussion or essay questions\n- For nonfiction:\n• A summary of the main concepts\n• Key terms or principles\n• 3 quiz questions (multiple choice or short answer)\nc. If the notes don’t seem to come from a book, provide a helpful summary or commentary on the content that could still support learning or reflection.\n\nIf no notes are provided, kindly prompt the user to share their notes, or ask what book they are reading so you can assist accordingly.\n\nAlways be helpful and flexible based on the input."
async function generateAIMaterials() {
	console.log("Generating AI Response...");
	var notesMessage = message_header;
	for (var i = 0; i < notesList.length; i++) {
		notesMessage += "\n\nPage: " + notesPageNumberList[i] + "\n";
		notesMessage += notesList[i];
	}
	console.log("Start OpenAI Query");
	var response = await queryOpenAI(notesMessage);
	console.log("Query Finished!");
	document.getElementById("aiResponseParagraph").textContent = response;
	console.log("AI Response Generated!");
}

// ------------------ FILE HANDLER STUFF ---------------------------------

function FH_TestFunction() {
    const fs = require("fs");
    console.log(" Writing into an file ");
    
    // Sample.txt is an empty file
    fs.writeFile(
        "sample.txt",
        "Let's write a few sentences in the file",
        function (err) {
            if (err) {
                return console.error(err);
            }
    
            // If no error the remaining code executes
            console.log(" Finished writing ");
            console.log("Reading the data that's written");
    
            // Reading the file
            fs.readFile("sample.txt", function (err, data) {
                if (err) {
                    return console.error(err);
                }
                console.log("Data read : " + data.toString());
    
            });
        }
    );
}

