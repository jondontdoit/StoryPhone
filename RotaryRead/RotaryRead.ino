
int timeout = 200; // time in ms for signal to be blank


#define INPUT_PIN 0

int rotaryStateLast;
unsigned long lastChange;
int pulseCount;


void setup() {
  Serial.begin(115200);

  pinMode(INPUT_PIN, INPUT_PULLUP);
}


void loop() {
  unsigned long now = millis();  
  int rotaryState = digitalRead(INPUT_PIN);

  if (rotaryState != rotaryStateLast) {
    pulseCount++;
    lastChange = now;
    rotaryStateLast = rotaryState;
  }

  if (now - lastChange > timeout && pulseCount > 0) {
    Serial.println(pulseCount/2);
    pulseCount = 0;
  }
  
  delay(1);        // delay in between reads for stability
}
