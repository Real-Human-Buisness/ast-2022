#include <Arduino.h>

#include <MD_YX5300.h>

#define SERIAL_COUNT 3
HardwareSerial *serials[3] = {&Serial1, &Serial3, &Serial5};
MD_YX5300 *mp3s[SERIAL_COUNT];

void cbResponse(const MD_YX5300::cbData *status)
// Used to process device responses either as a library callback function
// or called locally when not in callback mode.
{
  Serial.print(F("\nSync Status: "));

  switch (status->code)
  {
  case MD_YX5300::STS_OK:         Serial.print(F("STS_OK"));         break;
  case MD_YX5300::STS_TIMEOUT:    Serial.print(F("STS_TIMEOUT"));    break;
  case MD_YX5300::STS_VERSION:    Serial.print(F("STS_VERSION"));    break;
  case MD_YX5300::STS_CHECKSUM:   Serial.print(F("STS_CHECKSUM"));    break;
  case MD_YX5300::STS_TF_INSERT:  Serial.print(F("STS_TF_INSERT"));  break;
  case MD_YX5300::STS_TF_REMOVE:  Serial.print(F("STS_TF_REMOVE"));  break;
  case MD_YX5300::STS_ERR_FILE:   Serial.print(F("STS_ERR_FILE"));   break;
  case MD_YX5300::STS_ACK_OK:     Serial.print(F("STS_ACK_OK"));     break;
  case MD_YX5300::STS_FILE_END:   Serial.print(F("STS_FILE_END"));   break;
  case MD_YX5300::STS_INIT:       Serial.print(F("STS_INIT"));       break;
  case MD_YX5300::STS_STATUS:     Serial.print(F("STS_STATUS"));     break;
  case MD_YX5300::STS_EQUALIZER:  Serial.print(F("STS_EQUALIZER"));  break;
  case MD_YX5300::STS_VOLUME:     Serial.print(F("STS_VOLUME"));     break;
  case MD_YX5300::STS_TOT_FILES:  Serial.print(F("STS_TOT_FILES"));  break;
  case MD_YX5300::STS_PLAYING:    Serial.print(F("STS_PLAYING"));    break;
  case MD_YX5300::STS_FLDR_FILES: Serial.print(F("STS_FLDR_FILES")); break;
  case MD_YX5300::STS_TOT_FLDR:   Serial.print(F("STS_TOT_FLDR"));   break;
  default: Serial.print(F("STS_??? 0x")); Serial.print(status->code, HEX); break;
  }

  Serial.print(F(", 0x"));
  Serial.print(status->data, HEX);
}


void start_all() {
    for (int i = 0; i < SERIAL_COUNT; i++) {
        mp3s[i]->playTrackRepeat(0);
    }
}

void stop_all() {
    for (int i = 0; i < SERIAL_COUNT; i++) {
        mp3s[i]->playStop();
    }
}

void setup_and_play(void) {
    // setup players
    for (int i = 0; i < SERIAL_COUNT; i++) {
        serials[i]->begin(MD_YX5300::SERIAL_BPS);
        mp3s[i] = new MD_YX5300(*serials[i]);
        mp3s[i]->setSynchronous(false);
        mp3s[i]->volume(255);
    }
    // wait to start them all so the other message gets processed
    delay(1000);
    start_all();
    delay(1000);
}
MD_YX5300 mp3(Serial1);

void setup() {
    Serial.begin(9600);
    setup_and_play();
    digitalWrite(13, true);
}

void check_all() {
    for (int i = 0; i < SERIAL_COUNT; i++) {
        mp3s[i]->check();
    }
}

long last_stop_millis = 0;
bool is_playing = true;

void loop() {
    // check_all();
    if (millis() - last_stop_millis > 30000) {
        stop_all();
        delay(200);
        start_all();
        is_playing = !is_playing;
        digitalWrite(13, !digitalRead(13));
        last_stop_millis = millis();
    }
}