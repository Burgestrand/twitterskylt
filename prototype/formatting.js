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

function displayLines(lines) {
	var message = "";
	lines.forEach(function(line) {
		message += line.join(" ").padRight(" ", LINE_LENGTH);
	});
	console.log(displayFriendly(message));
}

String.prototype.words = function() {
	return this.trim().split(/\s+/);
}

String.prototype.padRight = function(character, length) {
	var str = this;
	while ( str.length < length ) {
		str += character;
	}
	return str;
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
		currentLength += word.length;
		currentLength += 1; // 1 represents the space character that must separate two words.
		currentLine().push(word);
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
			var lineCharsLeft = LINE_LENGTH - currentLength;
			
			// If we can move this word down and still have enough space left, we do so.
			// This also requires that the word is short enough to fit on a single line.
			// We must also do this if there are no chars left on the current line, leaving us no choice.
			if ( (minimumCharsNeeded(wordsLeft) <= charsLeft && word.length <= LINE_LENGTH) || lineCharsLeft <= 0 ) {
				addLine();
				addWord(word);
			}
			// Otherwise we must let this word run over two lines.
			else {
				var firstHalf = word.substring(0, lineCharsLeft);
				var secondHalf = word.substring(lineCharsLeft);
				
				addWord(firstHalf);
				addLine();
				words.splice(wordIndex + 1, 0, secondHalf); // Add it to the list of unprocessed words, so that words spanning multiple lines will be processed again as needed.
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