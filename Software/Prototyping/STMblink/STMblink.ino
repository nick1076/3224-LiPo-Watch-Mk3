


void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  delay(2000);
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(3, OUTPUT);
  Serial.println("Began");
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(3, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(3, LOW);
  delay(1000);
}
