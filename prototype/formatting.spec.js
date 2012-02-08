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
		expect(justifyGreedy("aaaaaa bbb cccc ddddd eeeeeee ffffff ggggg hhh iiiii j kk lllllll mmm nnnnnnn oo ppppp qqqqqqq rrrrrrrrrrrr ssss ttttt uuuuuuu vvvvv xxxx yyyyyy zzzz aaa bbbbb"))
		.toEqual([
			["aaaaaa", "bbb", "cccc", "ddddd", "eeeeeee", "ffffff"],    
			["ggggg", "hhh", "iiiii", "j", "kk", "lllllll", "mmm", "nnnnnnn"],
			["oo", "ppppp", "qqqqqqq", "rrrrrrrrrrrr", "ssss", "ttttt"],
			["uuuuuuu", "vvvvv", "xxxx", "yyyyyy", "zzzz", "aaa", "bbbbb"]
		]);
	});
	
	it("properly breaks up a word that is too large to fit on a single line", function() {
		expect(justifyGreedy("01234567890123456789012345678901234567890123456789"))
		.toEqual([
			["0123456789012345678901234567890123456789"],
			["0123456789"]
		]);
	});
});