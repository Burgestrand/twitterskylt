//int data = 7;
//int led = 13;

void setup() {
  Serial.begin(9600);
  //pinMode(data, INPUT); 
  //pinMode(led, OUTPUT);
}

void loop() {
/*
if (Serial.available() > 0) {
 data= Serial.read();
 if (data== 'H'){
 Serial.print(data, BYTE);
 digitalWrite(led, HIGH); 
 } 
 }
 else{
 digitalWrite(led, LOW); 
 }
 */
 Serial.print("THIS IS A TWEET \n THIS IS A NEW LINE");
 delay(1000);
 //xdelay(2000);
 //delay(1000);
}
