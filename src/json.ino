#include "yajl_parse.h"

static int null_event(void *context) { Serial.println("Found null."); }
static int bool_event(void *context, int value) { Serial.print("Found boolean: "); Serial.println(value); }
static int int_event(void *context, long long value) { Serial.print("Found integer: "); Serial.println((int) value); }
static int double_event(void *context, double value) { Serial.print("Found double: "); Serial.println(value); }
static int string_event(void *context, const unsigned char *value, size_t length) {
	Serial.print("Found string: ");
	Serial.write(value, length);
}

static yajl_callbacks callbacks = {
	null_event,
	bool_event,
	NULL,
	double_event,
	NULL,
	string_event,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static const unsigned char json[] = 
	"{"
	"\"page\": 1,"
	"\"query\": \"from%3Aajstream\","
	"\"results\": ["
	"{"
	"\"created_at\": \"Thu, 01 Mar 2012 14:30:13 +0000\","
	"\"geo\": null,"
	"\"id\": 175226439505936400,"
	"\"metadata\": {"
	"\"result_type\": \"recent\""
	"},"
	"\"text\": \"What are some issues facing voters in #Iran? Tell #AJStream what you think or record a short vid here:http://t.co/VUpymlxD\""
	"}"
	"],"
	"\"results_per_page\": 1,"
	"\"since_id\": 0"
	"}";

static unsigned char *message;
static unsigned char *twitter_date;

void setup() {
	Serial.begin(9600);
	
	yajl_handle handle = yajl_alloc(&callbacks, NULL, NULL);
	yajl_status status = yajl_parse(handle, json, 345);
	if (status != 0) {
		char *error = (char*) yajl_get_error(handle, 1, json, 345);
		Serial.print("Error: ");
		Serial.println(error);
	}
	
	yajl_free(handle);
	
	Serial.print("Status: ");
	Serial.println(status);
}

void loop() {
}
