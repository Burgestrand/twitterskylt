describe("displayFriendly", function() {
	it("only prints 164 characters (including 4 newlines)", function() {
		var longMessage = "";
		for ( var i = 0; i < 170; i++ ) {
			longMessage += "a";
		}
		expect(displayFriendly(longMessage).length).toEqual(164);
	});
	
	it("always prints 164 characters (including 4 newlines)", function() {
		expect(displayFriendly("a").length).toEqual(164);
	});
	
	it("properly handles empty strings", function() {
		expect(displayFriendly("").length).toEqual(164);
	});
});

describe("words", function() {
	it("splits text into words", function() {
		expect("You shall not pass!".words()).toEqual(["You", "shall", "not", "pass!"]);
	});
	
	it("collapses whitespace", function() {
		expect("Hello   there".words()).toEqual(["Hello", "there"]);
	});
	
	it("removes leading and trailing whitespace", function() {
		expect("   good bye   ".words()).toEqual(["good", "bye"]);
	});
});

describe("justifyGreedy", function() {
	it("properly justifies a single broken word", function() {
		expect(justifyGreedy("this is a tweet about apples, pears, bananas"))
		.toEqual([
			["this", "is", "a", "tweet", "about", "apples,", "pears,"],
			["bananas"]
		]);
	});
	
	it("properly justifies a single broken word followed by more words", function() {
		expect(justifyGreedy("this is a tweet about apples, pears, bananas, mangoes in syrup"))
		.toEqual([
			["this", "is", "a", "tweet", "about", "apples,", "pears,"],
			["bananas,", "mangoes", "in", "syrup"]
		]);
	});
	
	it("does not justify a word that fits exactly at the end of a line", function() {
		expect(justifyGreedy("a tweet that exactly fits the first line of the display"))
		.toEqual([
			["a", "tweet", "that", "exactly", "fits", "the", "first", "line"],
			["of", "the", "display"]
		]);
	});
	
	it("properly justifies tweets with broken words on multiple lines", function() {
		expect(justifyGreedy("this is a sample tweet with multiple words broken: on the first line and additionally on the second line"))
		.toEqual([
			["this", "is", "a", "sample", "tweet", "with", "multiple"],
			["words", "broken:", "on", "the", "first", "line", "and"],
			["additionally", "on", "the", "second", "line"]
		]);
	});
	
	it("does not justify a word if that would make the result too long", function() {
		// In this example, only the first broken word should be justified; the rest should be split over two lines.
		expect(justifyGreedy("#SCAF's Tantawi has met today w the #US Joint Chief of Staff Gen. Martin Dempsey to discuss military ties and the #NGO situation in #Egypt and #Syria and #Yemen"))
		.toEqual([
			["#SCAF's", "Tantawi", "has", "met", "today", "w", "the", "#US"],    
			["Joint", "Chief", "of", "Staff", "Gen.", "Martin", "Dempsey"],
			["to", "discuss", "military", "ties", "and", "the", "#NGO", "si"],
			["tuation", "in", "#Egypt", "and", "#Syria", "and", "#Yemen"]
		]);
	});
	
	it("properly breaks up a word that is too large to fit on a single line", function() {
		expect(justifyGreedy("01234567890123456789012345678901234567890123456789"))
		.toEqual([
			["0123456789012345678901234567890123456789"],
			["0123456789"]
		]);
	});
	
	it("properly breaks up a word that is too large to fit on multiple lines", function() {
		expect(justifyGreedy("012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"))
		.toEqual([
			["0123456789012345678901234567890123456789"],
			["0123456789012345678901234567890123456789"],
			["0123456789"]
		]);
	});
	
	it("properly handles the maximum number of words", function() {
		expect(justifyGreedy("aa a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a"))
		.toEqual([
			["aa",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a"], 
			["a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a"],  
			["a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a"],  
			["a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a",  "a"]
		]);
	});
	
	it("properly handles the maximum tweet length", function() {
		expect(justifyGreedy("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"))
		.toEqual([
			["aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"],
			["aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"],
			["aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"],
			["aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"]
		]);
	});
	
	it("properly moves down a word at the end of a line in a too long tweet", function() {
		expect(justifyGreedy("12 1234 123456789 abc def gh ijk 1234567 123456 123 123 12345678999 abcdefghijklmn abcdefghijklmn abcdefghijklmn abcdefghijklmn abcdefghijklabcdefghijklabcdefghijkl"))
		.toEqual([
			["12", "1234", "123456789", "abc", "def", "gh", "ijk", "1234567"],
			["123456",  "123",  "123",  "12345678999", "abcdefghijklm"],
			["n", "abcdefghijklmn", "abcdefghijklmn", "abcdefgh"],
			["ijklmn", "abcdefghijklabcdefghijklabcdefghi"],
			["jkl"]
		]);
	});
});