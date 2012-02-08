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

// Returns a list of lines that in turn consist of a list of words to be displayed on that line.
// Justifies the message by simply moving any broken words down to the next line if there is room.
function justifyGreedy(message) {
	var words = message.words();
	var lines = [];
	var currentLength = 0;
	
	function addLine() {
		lines.push([]);
		currentLength = 0;
	}
	
	function currentLine() {
		return lines[lines.length - 1];
	}
	
	function addWord(word) {
		currentLine().push(word);
		currentLength += word.length + 1; // 1 represents the space character that must separate two words.
	}
	
	addLine();
	
	for ( var wordIndex = 0; wordIndex < words.length; wordIndex++ ) {
		var word = words[wordIndex];
		
		// If the word fits the current line, we add it.
		if ( currentLength + word.length <= LINE_LENGTH ) {
			addWord(word);
		}
		// Otherwise, the current word exceeds the line length and we must handle it.
		else {
			var linesLeft = LINE_COUNT - lines.length;
			var charsLeft = linesLeft * LINE_LENGTH;
			var wordsLeft = words.slice(wordIndex);
			
			// If we can move this word down and still have enough space left, we do so.
			if ( minimumCharsNeeded(wordsLeft) <= charsLeft ) {
				addLine();
				addWord(word);
			}
			// Otherwise we must let this word run over two lines.
			else {
				var lineCharsLeft = LINE_LENGTH - currentLength - 1; // 1 for the space character that must precede this word.
				var firstHalf = word.substring(0, lineCharsLeft);
				var secondHalf = word.substring(lineCharsLeft);
				
				addWord(firstHalf);
				addLine();
				addWord(secondHalf);
			}
		}
	}
	
	return lines;
}

// Returns the minimum number of characters needed to display a list of words (including spaces) on a number of lines.
function minimumCharsNeeded(words) {
	var currentLength = 0;
	for ( var wordIndex = 0; wordIndex < words.length; wordIndex++ ) {
		var word = words[wordIndex];
		currentLength += word.length;
		var wordEndsAtLineEnd = currentLength % LINE_LENGTH === 0;
		if ( !wordEndsAtLineEnd ) {
			currentLength += 1; // Space character needed after word.
		}
	}
	
	return currentLength;
}