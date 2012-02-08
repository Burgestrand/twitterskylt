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