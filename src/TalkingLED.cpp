/* .+

.context    : Arduino Utility Libraries
.title      : Talking LED Library
.kind       : c++ source
.author     : Fabrizio Pollastri <mxgbot@gmail.com>
.site       : Revello - Italy
.creation   : 3-Dec-2018
.copyright  : (c) 2018 Fabrizio Pollastri

.description

  Make your system to display messages by LED blinking
  With this library, your on board LED can display a lot
  of useful messages. Each message is rendered by an
  on, off sequence with a specific timing.
  Sequences can be user defined or generated by a builtin code.
  Builtin sequences are able to display 15 different
  message codes.

.- */

#include <TalkingLED.h>


TalkingLED::TalkingLED(void) {
  LEDPin = LED_PIN;
  LEDStatus = TLED_OFF;
  digitalWrite(LEDPin,LEDStatus);
  pinMode(LEDPin,OUTPUT);
  messageCodeNext = 0;
  messageCodeCurrent = 0;
  sequenceNext = NULL;
  sequenceCurrent = NULL;
  sequence = NULL;
  sequenceEnd = false;
}


bool TalkingLED::begin() {
  return true;
}


bool TalkingLED::begin(uint8_t aLEDPin) {
  LEDPin = aLEDPin;
  return true;
}


bool TalkingLED::setMessage(uint8_t aMessageCode,
  TalkingLEDMessageType aMessageType) {
  messageCodeNext = aMessageCode;
  messageTypeNext = aMessageType;
  return _build_message_sequence();
}


bool TalkingLED::setSequence(uint16_t *aSequence) {
  sequenceNext = aSequence;
  return true;
}


bool TalkingLED::update(void) {
  now = millis();
  if (sequence) {
    if (now < nextChangeTime)
      return false;
    else {
      if (*sequence) {
        LEDStatus ^= 0x1;
        digitalWrite(LEDPin,LEDStatus);
        nextChangeTime = now + *sequence++;
        sequenceEnd = false;
        return true;
      }
      else
	sequenceEnd = true;
    }
  }
  if (sequenceNext) {
    sequenceCurrent = sequenceNext;
    sequenceNext = NULL;
  }
  sequence = sequenceCurrent;
  nextChangeTime = now;
  return false;
}


void TalkingLED::waitEnd() {
  while (!sequenceEnd) {
    update();
    ::delay(TLED_DELAY_STEP);
  }
  sequenceEnd = false;
}


void TalkingLED::delay(uint32_t aDelay) {
  delayEnd = aDelay + millis();
  while (delayEnd - millis() > TLED_DELAY_STEP) {
    update();
    ::delay(TLED_DELAY_STEP);
  }
  update();
  ::delay(delayEnd - millis());
}


void TalkingLED::setLED(uint8_t aLEDStatus) {
  LEDStatus = aLEDStatus;
  digitalWrite(LEDPin,LEDStatus);
  sequenceNext = NULL;
  sequenceCurrent = NULL;
  sequence = NULL;
}


bool TalkingLED::_build_message_sequence() {
  uint8_t j,k;
  uint8_t mask;
  sequenceNext = messageSequence + 6;
  switch(messageTypeNext) {
    case TLED_MORSE:
      if (messageCodeNext > TLED_MORSE_CODE_MAX)
        return false;
      for (j=0; j < messageCodeNext / TLED_LONG_BLINK_UNITS; j++) {
        *sequenceNext++ = TLED_LONG_BLINK_ON_TIME;
        *sequenceNext++ = TLED_LONG_BLINK_OFF_TIME;  
      }
      for (j=0; j < messageCodeNext % TLED_LONG_BLINK_UNITS; j++) {
        *sequenceNext++ = TLED_SHORT_BLINK_ON_TIME;
        *sequenceNext++ = TLED_SHORT_BLINK_OFF_TIME;
      }
      break;
     case TLED_BYTE:
      mask = 0x80;
      for (k=0; k < 2; k++) {
        for (j=0; j < 4; j++) {
          if (messageCodeNext & mask) {
            *sequenceNext++ = TLED_LONG_BLINK_ON_TIME;
            *sequenceNext++ = TLED_LONG_BLINK_OFF_TIME;
          }
          else {
            *sequenceNext++ = TLED_SHORT_BLINK_ON_TIME;
            *sequenceNext++ = TLED_SHORT_BLINK_OFF_TIME;
	  }
          mask >>= 1;
        }
	*(--sequenceNext)++ = TLED_NIBBLE_OFF_TIME;
      }
      break;
     case TLED_NIBBLE:
      mask = 0x08;
      for (j=0; j < 4; j++) {
        if (messageCodeNext & mask) {
          *sequenceNext++ = TLED_LONG_BLINK_ON_TIME;
          *sequenceNext++ = TLED_LONG_BLINK_OFF_TIME;
        }
        else {
          *sequenceNext++ = TLED_SHORT_BLINK_ON_TIME;
          *sequenceNext++ = TLED_SHORT_BLINK_OFF_TIME;
        }
        mask >>= 1;
      }
      break;
    default:
      return false;
  }
  *--sequenceNext = TLED_MESSAGE_END_OFF_TIME;
  *++sequenceNext = 0;
  sequenceNext = messageSequence;
  return true;
}

/**** END ****/
