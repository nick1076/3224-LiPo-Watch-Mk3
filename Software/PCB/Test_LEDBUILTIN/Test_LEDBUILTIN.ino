
#define OUT_LEDBUILTIN PA5
#define OUT_INDICATORENA PB0

void setup() {
  pinMode(OUT_LEDBUILTIN, OUTPUT);
  pinMode(OUT_INDICATORENA, OUTPUT);
  delay(500);
  digitalWrite(OUT_INDICATORENA, HIGH);
}

void loop() {
  delay(1000);
  digitalWrite(OUT_LEDBUILTIN, HIGH);
  delay(1000);
  digitalWrite(OUT_LEDBUILTIN, LOW);
}
