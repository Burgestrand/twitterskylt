var LINE_LENGTH = 40;
var LINE_COUNT = 4;

function displayFriendly(message) {
	message = message.replace(/ /g, "_");
	var output = "";
	var charIndex = 0;
	for ( var line = 0; line < LINE_COUNT; line++ ) {
		for ( var column = 0; column < LINE_LENGTH; column++ ) {
			if ( message.charAt(charIndex) === "" ) {
				output += "*";
			} else {
				output += message.charAt(charIndex);
				charIndex++;
			}
		}
		output += "\n";
	}
	
	return output;
}

String.prototype.words = function() {
	return this.trim().split(/\s+/);
}

// Returns a list of lines, which in turn consist of a list of words to be displayed on that line.
// Justifies the message by simply moving any broken words down to the next line if there is room.
function justifyGreedy(message) {
	var words = message.words();
	var lines = [];
	var currentLine = [];
	var currentLength = 0;
	
	for ( var wordIndex = 0; wordIndex < words.length; wordIndex++ ) {
		var word = words[wordIndex];
		currentLength += word.length;
		// If the current word exceds the line length, we create a new line to start adding words to.
		if ( LINE_LENGTH < currentLength ) {
			lines.push(currentLine);
			currentLine = [];
			currentLength = 0; // We start counting from the beginning on the next line.
		// Otherwise, we need to 1 to the length to take into the account the whitespace that must follow a word.
		} else {
			currentLength += 1;
		}
		currentLine.push(word);
	}
	// We must also include the last line in the result.
	lines.push(currentLine);
	
	return lines;
}