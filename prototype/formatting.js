var lineLength = 40;
var lineCount = 4;

function displayFriendly(message) {
	var output = "";
	var charIndex = 0;
	for ( var line = 0; line < lineCount; line++ ) {
		for ( var column = 0; column < lineLength; column++ ) {
			if ( message.charAt(charIndex) === "" ) {
				output += "_";
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