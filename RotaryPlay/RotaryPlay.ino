#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

//////////////////
// GLOBALS
//////////////////
int timeout = 200; // time in ms for signal to be blank
boolean debug = false;
char *filenames[] = { 
  "/000000~1.MP3",
  "/000001~1.MP3",
  "/000002~1.MP3",
  "/000003~1.MP3",
  "/000004~1.MP3",
  "/000005~1.MP3",
  "/000006~1.MP3",
  "/000007~1.MP3",
  "/000008~1.MP3",
  "/000009~1.MP3",
  "/000010~1.MP3"
};

//////////////////
// PINS
//////////////////
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)
#define VS1053_CS       6     // VS1053 chip select pin (output)
#define VS1053_DCS     10     // VS1053 Data/command select pin (output)
#define CARDCS          5     // Card chip select pin
#define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin

#define ROTARY_INPUT_PIN 3
//#define CRADLE_INPUT_PIN 4


//////////////////
// INTERNAL
//////////////////

Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

int rotaryStateLast, cradleStateLast = 1;
unsigned long lastChange;
int pulseCount;

//////////////////
// SETUP
//////////////////
void setup() {
  if (debug) Serial.begin(115200);
  
  pinMode(ROTARY_INPUT_PIN, INPUT_PULLUP);
//  pinMode(CRADLE_INPUT_PIN, INPUT_PULLUP);

  randomSeed(A0);

  if (! musicPlayer.begin()) { // initialise the music player
     if (debug) Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }

  if (debug) Serial.println(F("VS1053 found"));
  
  if (!SD.begin(CARDCS)) {
    if (debug) Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  if (debug) Serial.println("SD OK!");
  if (debug) printDirectory(SD.open("/"), 0);
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(10,0);

  // If DREQ is on an interrupt pin we can do background audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);
 
  if (debug) musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
  
  if (debug) Serial.println(F("Hello!"));
  musicPlayer.startPlayingFile(filenames[0]);
}

//////////////////
// LOOP
//////////////////
void loop() {
  unsigned long now = millis();  
  int rotaryState = !digitalRead(ROTARY_INPUT_PIN);
  int cradleState = true; //!digitalRead(CRADLE_INPUT_PIN);

  // Catch pulses as they come in
  if (cradleState && rotaryState != rotaryStateLast) {
    pulseCount++;
    lastChange = now;
    rotaryStateLast = rotaryState;
  }

  // Once the pulses stop for longer than timeout, it's done
  if (cradleState && now - lastChange > timeout && pulseCount > 0) {
    pulseCount = pulseCount / 2;
    Serial.println(pulseCount);
    
    musicPlayer.stopPlaying();
    if (debug) {
      Serial.print("Playing ");
      Serial.println(filenames[pulseCount]);
    }
    musicPlayer.startPlayingFile(filenames[pulseCount]);

    pulseCount = 0;
  }

  // When the cradle is picked up
//  if (cradleState != cradleStateLast) {
//    musicPlayer.stopPlaying();
//    if (cradleState) {
//      if (debug) Serial.println(F("Hello!"));
//      musicPlayer.startPlayingFile(filenames[0]);
//    } else {
//      if (debug) Serial.println(F("Stopping!"));
//      pulseCount = 0;
//    }
//    cradleStateLast = cradleState;
//  }
  
  delay(1);        // delay in between reads for stability
}

//////////////////
// FUNCTIONS
//////////////////
/// File listing helper
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) break;
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}
