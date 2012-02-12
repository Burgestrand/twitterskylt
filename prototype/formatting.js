var LINE_LENGTH = 40;
var LINE_COUNT = 4;

String.prototype.words = function() {
	return this.trim().split(/\s+/);
}

function justifyGreedy(message) {
	var words = message.words();
	var lines = [];
	
	addLine();
	
	while ( words.length > 0 ) {
		if ( lineIsFull() ) {
			addLine();
		}
		
		if ( wordFitsLine(currentWord()) ) {
			addWord(currentWord());
			words.shift();
		}
		else if ( wordCanBeMovedDown(currentWord()) ) {
			addLine();
			addWord(currentWord());
			words.shift();
		}
		// Word must be broken.
		else {
			var firstPart = currentWord().substring(0, lineCharsLeft());
			var secondPart = currentWord().substring(lineCharsLeft());
			
			addWord(firstPart);
			addLine();
			
			words.shift();
			words.unshift(secondPart);
		}
	}
	
	return lines;
	
	function addLine() {
		lines.push([]);
	}
	
	function currentLine() {
		return lines[lines.length - 1];
	}
	
	function currentLength() {
		return currentLine().join(" ").length;
	}
	
	function addWord(word) {
		currentLine().push(word);
	}
	
	function currentWord() {
		return words[0];
	}
	
	function lineIsFull() {
		return (
			currentLength() === LINE_LENGTH || // No characters left at all.
			currentLength() === LINE_LENGTH - 1 // Only one character left, leaving no room for more words.
		);
	}
	
	function lineIsEmpty() {
		return currentLength() === 0;
	}
	
	function lineCharsLeft() {
		if ( lineIsEmpty() ) {
			return LINE_LENGTH - currentLength();
		} else {
			return LINE_LENGTH - currentLength() - " ".length;
		}
	}
	
	function wordFitsLine(word) {
		return word.length <= lineCharsLeft();
	}
	
	function wordCanBeMovedDown(word) {
		var roomNeeded = words.join(" ").length;
		var linesLeft = LINE_COUNT - lines.length;
		var roomLeft = linesLeft * LINE_LENGTH;
		var wordFitsSingleLine = word.length <= LINE_LENGTH;
		
		return roomNeeded <= roomLeft && wordFitsSingleLine;
	}
}