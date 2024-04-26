#include "Arduino.h"
#include <EEPROM.h>
#include "keyboard_decoder.h"

 // US layout modifier
const uint8_t KeyDecoder::numKeys1[10]    = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'}; // HID Scancode 0x1e - 0x27
const uint8_t KeyDecoder::numKeysAlt1[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'}; // HID Scancode 0x1e - 0x27 Numkeys with Alt
const uint8_t KeyDecoder::symKeysUp1[12]  = {'_', '+', '{', '}', '|',  '~', ':', '"',  '~', '<', '>', '?'}; //HID Scancode 0x2d - 0x38 with Shift
const uint8_t KeyDecoder::symKeysLo1[12]  = {'-', '=', '[', ']', '\\', 0x60, ';', '\'', '`', ',', '.', '/'}; //HID Scancode 0x2d - 0x38

// DE layout modifier
const uint8_t KeyDecoder::numKeys2[10]    = {'!', '"', '#', '$', '%', '&', '/', '(', ')', '='}; // HID Scancode 0x1e - 0x27 Numkeys with Shift
const uint8_t KeyDecoder::numKeysAlt2[10] = {'1', '²', '³', '4', '5', '6', '{', '[', ']', '}'}; // HID Scancode 0x1e - 0x27 Numkeys with Alt
const uint8_t KeyDecoder::symKeysUp2[12]  = {'?', '`', 'Ü', '*', '|',  '\'', 'Ö', 'Ä', '°', ';', ':', '_'}; //HID Scancode 0x2d - 0x38 with Shift
const uint8_t KeyDecoder::symKeysLo2[12]  = {'ß', '´', 'ü', '+', '\\', '#',  'ö', 'ä', '^', ',', '.', '-'}; //HID Scancode 0x2d - 0x38

uint8_t KeyDecoder::lastBuffer[21] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KeyDecoder::deltaK = 0x00;
uint8_t KeyDecoder::pos = 0;

KeyDecoder::KeyDecoder()
{
  uint8_t currentKeyboardLayout = 1;
  boolean capsLock = true;
}

void KeyDecoder::SetLayout(uint8_t layout)
{
  if (layout == 0) layout = 1;
  if (layout <= 2) currentLayout = layout;
}

uint8_t KeyDecoder::GetLayout(void)
{
  return currentLayout;
}

void KeyDecoder::ToggleCapsLock(void)
{
   capsLock = !capsLock;
}

void cpbuf(const uint8_t* src, uint8_t* dest, size_t len)
{
  for (size_t i = 0; i < len; ++i) {
      dest[i] = src[i];
  }
}

uint8_t findNewBit(uint8_t prevByte, uint8_t newByte)
{
  return prevByte ^ newByte;
}

uint8_t* findNewBitArray(uint8_t* prevBuffer, uint8_t* newBuffer)
{
  int length;
  for (length = 0; length < 21; (length)++) {
    if (prevBuffer[length] != newBuffer[length]) {
      return &newBuffer[length];
    }
  }
  return nullptr;
}

uint8_t KeyDecoder::Decode_NKRO(uint8_t *buf)
{
  uint8_t sCode = 0;
  boolean lrShift = false;
  uint8_t scanCode = 0xFF;
  uint8_t* newBit;

  newBit = findNewBitArray(lastBuffer, buf);

  if (newBit != nullptr) {
    pos = newBit - buf;
    deltaK = findNewBit(lastBuffer[pos], buf[pos]);
  }

  if (buf[pos] < lastBuffer[pos]) pos = 0; 
  if (buf[pos] == 0x00) pos = 0;

  cpbuf(buf, lastBuffer, 21);

  for (uint8_t j; j < 21;  j++){
    if (j>2){
      buf[j] = (j != pos) ? 0x00 : deltaK;
    }
  }

  if ((buf[1] & 0x02) || (buf[1] & 0x20)) lrShift = true; // L/R shift

  if (buf[3] & 0x01) sCode = 0x61; // a
  if (buf[3] & 0x02) sCode = 0x62; // b
  if (buf[3] & 0x04) sCode = 0x63; // c
  if (buf[3] & 0x08) sCode = 0x64; // d
  if (buf[3] & 0x10) sCode = 0x65; // e
  if (buf[3] & 0x20) sCode = 0x66; // f
  if (buf[3] & 0x40) sCode = 0x67; // g
  if (buf[3] & 0x80) sCode = 0x68; // h
  if (buf[4] & 0x01) sCode = 0x69; // i
  if (buf[4] & 0x02) sCode = 0x6A; // j
  if (buf[4] & 0x04) sCode = 0x6B; // k
  if (buf[4] & 0x08) sCode = 0x6C; // l
  if (buf[4] & 0x10) sCode = 0x6D; // m
  if (buf[4] & 0x20) sCode = 0x6E; // n
  if (buf[4] & 0x40) sCode = 0x6F; // o
  if (buf[4] & 0x80) sCode = 0x70; // p
  if (buf[5] & 0x01) sCode = 0x71; // q
  if (buf[5] & 0x02) sCode = 0x72; // r
  if (buf[5] & 0x04) sCode = 0x73; // s
  if (buf[5] & 0x08) sCode = 0x74; // t
  if (buf[5] & 0x10) sCode = 0x75; // u
  if (buf[5] & 0x20) sCode = 0x76; // v
  if (buf[5] & 0x40) sCode = 0x77; // w
  if (buf[5] & 0x80) sCode = 0x78; // x
  if (currentLayout == 2) {
    if (buf[6] & 0x02) sCode = 0x79; // z
    if (buf[6] & 0x01) sCode = 0x7A; // y
  } else {
    if (buf[6] & 0x01) sCode = 0x79; // y
    if (buf[6] & 0x02) sCode = 0x7A; // z
  }  

  if (sCode) {
    if (capsLock) sCode = sCode - 0x20; // capsLock
    else if (lrShift) sCode = sCode - 0x20; // L/R shift
    if ((buf[1] & 0x01) || (buf[1] & 0x10)) { // L/R ctrl
      if (sCode > 0x60) sCode = sCode - 0x60;
      else sCode = sCode - 0x40;
    }
  }
  
  if (buf[6] & 0x04) sCode = 0x31; // 1
  if (buf[6] & 0x08) sCode = 0x32; // 2
  if (buf[6] & 0x10) sCode = 0x33; // 3
  if (buf[6] & 0x20) sCode = 0x34; // 4
  if (buf[6] & 0x40) sCode = 0x35; // 5
  if (buf[6] & 0x80) sCode = 0x36; // 6
  if (buf[7] & 0x01) sCode = 0x37; // 7
  if (buf[7] & 0x02) sCode = 0x38; // 8
  if (buf[7] & 0x04) sCode = 0x39; // 9
  if (buf[7] & 0x08) { sCode = 0x30; // 0
    if (lrShift) {
      if (currentLayout == 2) sCode = KeyDecoder::numKeys2[9];
      else sCode = KeyDecoder::numKeys1[9];
    }  
  }
  if ((sCode > 0x30) && (sCode < 0x3A)) {
    if (lrShift) {
      if (currentLayout == 2) sCode = KeyDecoder::numKeys2[sCode - 0x31];
      else sCode = KeyDecoder::numKeys1[sCode - 0x31];
    }     
  }
    
  if (currentLayout == 1) {
    if (buf[15] & 0x01) { sCode = 0x5C; // backslash
      if (lrShift) sCode = 0x7C; // pipe
    }
  } else {
    if (buf[15] & 0x01) { sCode = 0x3C; // <
      if (lrShift) sCode = 0x3E; // >
    }
  }
  
  if (buf[8] & 0x02) scanCode = 0; // ß
  if (buf[8] & 0x04) scanCode = 1; // ´
  if (buf[8] & 0x08) scanCode = 2; // ü
  if (buf[8] & 0x10) scanCode = 3; // +
  if (buf[8] & 0x20) scanCode = 4; // 
  if (buf[8] & 0x40) scanCode = 5; // #
  if (buf[8] & 0x80) scanCode = 6; // ö
  if (buf[9] & 0x01) scanCode = 7; // ä
  if (buf[9] & 0x02) scanCode = 8; // ^
  if (buf[9] & 0x04) scanCode = 9; // ,
  if (buf[9] & 0x08) scanCode = 10; // .
  if (buf[9] & 0x10) scanCode = 11; // -
  if (scanCode < 12) { // sym key
    if (currentLayout == 1) sCode = KeyDecoder::symKeysLo1[scanCode];
    if (currentLayout == 2) sCode = KeyDecoder::symKeysLo2[scanCode];
    if (lrShift) { // shift
      if (currentLayout == 1) sCode = KeyDecoder::symKeysUp1[scanCode];
      if (currentLayout == 2) sCode = KeyDecoder::symKeysUp2[scanCode];
    }
  }

  if ((buf[1] & 0x01) && sCode) { // CTRL num
    if (buf[8] & 0x02) sCode = 0x5C; // backslash 
    if (buf[7] & 0x01) sCode = 0x7B; // { 
    if (buf[7] & 0x02) sCode = 0x5B; // [ 
    if (buf[7] & 0x04) sCode = 0x5D; // ] 
    if (buf[7] & 0x08) sCode = 0x7D; // } 
  }

  if (buf[7] & 0x10) sCode = 0x0D; // return
  if (buf[7] & 0x20) { sCode = 0x1B; // esc
    if (buf[1] & 0x01) sCode = 0x5E; // ^ 
  }
  if (buf[7] & 0x40) sCode = 0x08; // del
  if (buf[8] & 0x01) sCode = 0x20; // space
  if (buf[12] & 0x01) sCode = 0x08; // entf
  if (buf[12] & 0x08) sCode = 0x15; // ->
  if (buf[12] & 0x10) sCode = 0x08; // <-
  if (buf[12] & 0x20) sCode = 0x7F; // arrow down (ctrl + shift + <-)
  if (buf[1] & 0x04) sCode = 0x07; // alt
  if ((buf[1] & 0x40) && (buf[5] & 0x01)) sCode = 0x40; // (german alt-gr + q) @
  if ((buf[1] & 0x40) && (buf[15] & 0x01)) sCode = 0x7C; // (german alt-gr + <) pipe
  if (currentLayout == 2) { 
    if (buf[8] & 0x08) {
      sCode = 0x7E; // ü
    }
   if (buf[8] & 0x80) {
      sCode = 0x5B; // ö
     if (lrShift) sCode = 0x7B; // {  
   }
    if (buf[9] & 0x01) {
     sCode = 0x5D; // ä
     if (lrShift) sCode = 0x7D; // } 
    }
  }

  if (sCode > 0x80) sCode = 0;
  return sCode;
}

uint8_t KeyDecoder::Decode_USB(uint8_t *buf)
{
  uint8_t sCode = 0;
  boolean lrShift = false;
  uint8_t scanCode = 0xFF;

  if ((buf[0] & 0x02) || (buf[0] & 0x20)) { // L/R shift
    lrShift = true;
    buf[0] = 0x02;
  }

  if (buf[3] > 0x00) {
    buf[2] = buf[3];
    buf[3] = 0x00;
  }
  for (uint8_t i; i<7; i++) {
    if (buf[2] == lastBuffer[i]) buf[2] = 0x00;
  }
  cpbuf(buf, lastBuffer, 7);

  if ((buf[2] > 0x03) && (buf[2] < 0x1E)) { // letters
    sCode = buf[2] + 0x5D;
    if (currentLayout == 2) {
      if (sCode == 0x79) sCode = 0x7A; // y 
      else if (sCode == 0x7A) sCode = 0x79; // z 
    }  
    if (capsLock) sCode = sCode - 0x20; // capsLock
    else if (lrShift) sCode = sCode - 0x20; // shift
    if ((buf[0] == 0x01) || (buf[0] == 0x10)) { // L/R ctrl
      if (sCode > 0x60) sCode = sCode - 0x60;
      else sCode = sCode - 0x40;
    }
  } 
    
  if (buf[2] == 0x27) { sCode = 0x30; // 0
    if (lrShift) {
      if (currentLayout == 2) sCode = KeyDecoder::numKeys2[9];
      else sCode = KeyDecoder::numKeys1[9];
    }  
  }
  
  if ((buf[2] > 0x1D) && (buf[2] < 0x27)) { // 1-9
    sCode = buf[2] + 0x13;
    if (lrShift) {
      if (currentLayout == 1) sCode = KeyDecoder::numKeys1[sCode - 0x31]; // US
      if (currentLayout == 2) sCode = KeyDecoder::numKeys2[sCode - 0x31]; // DE
    }     
  }
  
  if (currentLayout == 1) {
    if (buf[2] == 0x64) { sCode = 0x5C; // backslash
      if (lrShift) sCode = 0x7C; // pipe
    }
  } else {
    if (buf[2] == 0x64) { sCode = 0x3C; // <
      if (lrShift) sCode = 0x3E; // >
    }
  }

  if (buf[2] == 0x2D) scanCode = 0; // ß
  if (buf[2] == 0x2E) scanCode = 1; // ´
  if (buf[2] == 0x2F) scanCode = 2; // ü
  if (buf[2] == 0x30) scanCode = 3; // +
  if (buf[2] == 0x31) scanCode = 4; // 
  if (buf[2] == 0x32) scanCode = 5; // #
  if (buf[2] == 0x33) scanCode = 6; // ö
  if (buf[2] == 0x34) scanCode = 7; // ä
  if (buf[2] == 0x35) scanCode = 8; // ^
  if (buf[2] == 0x36) scanCode = 9; // ,
  if (buf[2] == 0x37) scanCode = 10; // .
  if (buf[2] == 0x38) scanCode = 11; // -
  if (scanCode < 12) { // sym key
    if (currentLayout == 1) sCode = KeyDecoder::symKeysLo1[scanCode];
    if (currentLayout == 2) sCode = KeyDecoder::symKeysLo2[scanCode];
    if (lrShift) { // shift
      if (currentLayout == 1) sCode = KeyDecoder::symKeysUp1[scanCode];
      if (currentLayout == 2) sCode = KeyDecoder::symKeysUp2[scanCode];
    }
  }

  if (buf[2] == 0x28) sCode = 0x0D; // return
  if (buf[2] == 0x58) sCode = 0x0D; // keyPad return
  if (buf[2] == 0x29) sCode = 0x1B; // esc
  if (buf[2] == 0x2A) sCode = 0x10; // del
  if (buf[2] == 0x2C) sCode = 0x20; // space
  if (buf[2] == 0x4C) sCode = 0x08; // entf
  if (buf[2] == 0x4F) sCode = 0x15; // ->
  if (buf[2] == 0x50) sCode = 0x08; // <-
  if (buf[0] == 0x04) sCode = 0x07; // alt 
  if (buf[2] == 0x2A) sCode = 0x08; // backspace maps to <- 
  if (buf[2] == 0x51) sCode = 0x7F; // arrow down (ctrl + shift + <-) 


  if ((buf[0] == 0x01) && sCode) { // CTRL num
    if (buf[2] == 0x2D) sCode = 0x5C; // backslash
    if (buf[2] == 0x24) sCode = 0x7B; // { 
    if (buf[2] == 0x25) sCode = 0x5B; // [ 
    if (buf[2] == 0x26) sCode = 0x5D; // ] 
    if (buf[2] == 0x27) sCode = 0x7D; // } 
    if (buf[2] == 0x11) sCode = 0x80; // NUL 
  }

  if ((buf[0] == 0x40) && (buf[2] == 0x14)) sCode = 0x40; // german (alt GR + q) @
  if ((buf[0] == 0x40) && (buf[2] == 0x64)) sCode = 0x7C; // german (alt GR + <) pipe

  if (buf[2] == 0x62) sCode = 0x30; // keyPad 0 
  if ((buf[2] > 0x58) && (buf[2] < 0x62)) sCode = buf[2] - 0x28; // keyPad 1-9
  if (buf[2] == 0x54) sCode = 0x2F; // keyPad /
  if (buf[2] == 0x55) sCode = 0x2A; // keyPad *
  if (buf[2] == 0x56) sCode = 0x2D; // keyPad -
  if (buf[2] == 0x57) sCode = 0x3B; // keyPad +
  if (currentLayout == 1) if (buf[2] == 0x63) sCode = 0x2E; // keyPad .
  if (currentLayout == 2) if (buf[2] == 0x63) sCode = 0x2C; // keyPad ,

  if (sCode > 0x80) sCode = 0;
  return sCode;
}
