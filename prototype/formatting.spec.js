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
		var words = "You shall not pass!".words();
		expect(words[0]).toEqual("You");
		expect(words[1]).toEqual("shall");
		expect(words[2]).toEqual("not");
		expect(words[3]).toEqual("pass!");
	});
	
	it("collapses whitespace", function() {
		var words = "Hello   there".words();
		expect(words[0]).toEqual("Hello");
		expect(words[1]).toEqual("there");
	});
	
	it("removes leading and trailing whitespace", function() {
		var words = "   good bye   ".words();
		expect(words[0]).toEqual("good");
		expect(words[1]).toEqual("bye");
	});
});