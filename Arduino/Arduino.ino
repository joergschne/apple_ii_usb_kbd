/*
           _____  _____  _____  _      ______   _____ _____ 
      /\   |  __ \|  __ \|  __ \| |    |  ____| |_   _|_   _|
     /  \  | |__) | |__) | |__) | |    | |__      | |   | |  
    / /\ \ |  ___/|  ___/|  ___/| |    |  __|     | |   | |  
   / ____ \| |    | |    | |    | |____| |____   _| |_ _| |_ 
  /_/  __\_\_|____|_|____|_|____|______|______|_|_____|_____|
      |  ____/ __ \|  __ \|  ____\ \    / /  ____|  __ \     
      | |__ | |  | | |__) | |__   \ \  / /| |__  | |__) |    
      |  __|| |  | |  _  /|  __|   \ \/ / |  __| |  _  /     
      | |   | |__| | | \ \| |____   \  /  | |____| | \ \     
      |_|    \____/|_|  \_\______|   \/   |______|_|  \_\    

   APPLE ][ USB-Dual-Keyboard-Adapter (Arduino UNO ATMEAG328P)
                      Version: 04/2024
*/

#include <hidcomposite.h>
#include <usbhub.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <SPI.h>

#include "Simpletimer.h"

#include "keyboard_encoder.h"
#include "keyboard_decoder.h"

#define PrintDebugInfo true
#define DefaultKeyboardLayout 2 // 1=US 2=DE
#define maxLayout 2

#define REPEAT_INTERVAL_MS 80
#define REPEAT_DELAY 8
#define MAX_REPEAT_BUFFER 64

const char string_0[] PROGMEM = "            _____  _____  _____  _      ______   _____ _____ ";
const char string_1[] PROGMEM = "      /\\   |  __ \\|  __ \\|  __ \\| |    |  ____| |_   _|_   _|";
const char string_2[] PROGMEM = "     /  \\  | |__) | |__) | |__) | |    | |__      | |   | |  ";
const char string_3[] PROGMEM = "    / /\\ \\ |  ___/|  ___/|  ___/| |    |  __|     | |   | |  ";
const char string_4[] PROGMEM = "   / ____ \\| |    | |    | |    | |____| |____   _| |_ _| |_ ";
const char string_5[] PROGMEM = "  /_/  __\\_\\_|____|_|____|_|____|______|______|_|_____|_____|";
const char string_6[] PROGMEM = "      |  ____/ __ \\|  __ \\|  ____\\ \\    / /  ____|  __ \\     ";
const char string_7[] PROGMEM = "      | |__ | |  | | |__) | |__   \\ \\  / /| |__  | |__) |    ";
const char string_8[] PROGMEM = "      |  __|| |  | |  _  /|  __|   \\ \\/ / |  __| |  _  /     ";
const char string_9[] PROGMEM = "      | |   | |__| | | \\ \\| |____   \\  /  | |____| | \\ \\     ";
const char string_A[] PROGMEM = "      |_|    \\____/|_|  \\_\\______|   \\/   |______|_|  \\_\\    ";
const char *const string_table[] PROGMEM = { string_0, string_1, string_2, string_3, string_4, string_5, string_6, string_7, string_8, string_9, string_A };
char buffer[80]; // make sure this is large enough for the largest string it must hold

uint8_t repeatBuffer[MAX_REPEAT_BUFFER] = {};

uint8_t lastKeyPressed = 0;
uint8_t repeatDelay = REPEAT_DELAY;
uint8_t repeatBufferIndex = 0;

boolean lodeRunnerMapping = false;

KeyEncoder KeyEncoder;
KeyDecoder CurrentLayout;
Simpletimer timer1{};

void timerCallback() {
  if (lastKeyPressed){
    if (repeatDelay == 0){
      Serial.println("Autorepeat!");
      KeyEncoder.IIstrobe();
    } else repeatDelay --;
  } else repeatDelay = REPEAT_DELAY;
}

void add2Buffer() {
  repeatBuffer[repeatBufferIndex] = lastKeyPressed;
  if (lastKeyPressed == 0x0D) repeatBufferIndex = 0;
  else if (repeatBufferIndex < MAX_REPEAT_BUFFER) repeatBufferIndex ++;
}

void printBuffer(){
  repeatBufferIndex = 0;
  while ((repeatBuffer[repeatBufferIndex] != 0x0D) && (repeatBufferIndex < MAX_REPEAT_BUFFER)) {
    KeyEncoder.IIputchar((char)repeatBuffer[repeatBufferIndex]);
    delay(50);
    repeatBufferIndex ++;
  }  
}

// Override HIDComposite to be able to select which interface we want to hook into
class HIDSelector : public HIDComposite {
public:
  HIDSelector(USB *p)
    : HIDComposite(p){};

protected:
  void ParseHIDData(USBHID *hid, uint8_t ep, bool is_rpt_id, uint8_t len, uint8_t *buf);  // Called by the HIDComposite library
  bool SelectInterface(uint8_t iface, uint8_t proto);
};

// Return true for the interface we want to hook into
bool HIDSelector::SelectInterface(uint8_t iface, uint8_t proto) {
  if (proto != 0) return true;
  return false;
}

// Will be called for all HID data received from the USB interface
void HIDSelector::ParseHIDData(USBHID *hid, uint8_t ep, bool is_rpt_id, uint8_t len, uint8_t *buf) {

  uint8_t keyCode = 0;

#if PrintDebugInfo
  if (len && buf) {
    Serial.print("USB debug: ");
    for (uint8_t i = 0; i < len; i++) {
      Serial.print(i);
      if (buf[i] == 0x00) Serial.print("-");
      else Serial.print("#");
      if (buf[i] < 0x10) Serial.print("0");
      Serial.print((buf[i] & 0xFF), HEX);
      Serial.print(" ");
    }
    Serial.println("");
  }
#endif
  
  if (len == 21) { // Cherry N-key-rollover USB-keyboard detected
    if (buf[9] & 0x20) CurrentLayout.ToggleCapsLock(); // capsLock
    //if (buf[12] & 0x20) // arrow down
    if (buf[12] & 0x40) printBuffer(); // arrow up
    if (buf[1] & 0x08) {
      KeyEncoder.Disable();
      if (buf[6] & 0x04) { // win 1 = switch keyboard layout to US
        CurrentLayout.SetLayout(1);
        lodeRunnerMapping = false;
        Serial.println("Keyboard layout: US");
      }
      if (buf[6] & 0x08) { // win 2 = switch keyboard layout to DE
        CurrentLayout.SetLayout(2);
        lodeRunnerMapping = false;
        Serial.println("Keyboard layout: DE");
      }
      if (buf[7] & 0x04) { // win 9 = special WASD (NM) mapping for LodeRunner
        lodeRunnerMapping = true;
        Serial.println("Keyboard layout: LodeRunner WASD (NM) special");
      }
      if (buf[7] & 0x08) { // win 0 = save default keyboard
        EEPROM.write(0, CurrentLayout.GetLayout());
        Serial.println("Keyboard layout saved!");
      }  
      buf[6] = 0;
      buf[7] = 0;
    }
    if ((buf[1] & 0x01) && (buf[11] & 0x20)) { // reset 
      KeyEncoder.IIreset();
      Serial.println("Reset!");
    }
    keyCode = CurrentLayout.Decode_NKRO(buf);
    if (keyCode) {
      lastKeyPressed = keyCode;
      add2Buffer();
      if (lodeRunnerMapping) {
        if (keyCode == 0x57) keyCode = 0x49; // W
        if (keyCode == 0x41) keyCode = 0x4A; // A
        if (keyCode == 0x53) keyCode = 0x4B; // S
        if (keyCode == 0x44) keyCode = 0x4C; // D
        if (keyCode == 0x4E) keyCode = 0x55; // N
        if (keyCode == 0x4D) keyCode = 0x4F; // M
      }
      KeyEncoder.IIputchar((char)keyCode);
      Serial.print("NKRO keyCode: 0x");
      Serial.print(keyCode, HEX);
      Serial.println(" ");
    } else lastKeyPressed = 0;
  
 } else { 
  
    if (len == 8) { // other USB-keyboard detected  
      if (buf[2] == 0x39) CurrentLayout.ToggleCapsLock(); // capsLock
      //if (buf[2] == 0x51) // arrow down
      if (buf[2] == 0x52) printBuffer(); // arrow up
      if (buf[0] == 0x08) {
       KeyEncoder.Disable();
       if (buf[2] == 0x1E) { // win 1 = switch keyboard layout to US
         CurrentLayout.SetLayout(1);
         lodeRunnerMapping = false;
         Serial.println("Keyboard layout: US");
       }
       if (buf[2] == 0x1F) { // win 2 = switch keyboard layout to DE
         CurrentLayout.SetLayout(2);
         lodeRunnerMapping = false;
         Serial.println("Keyboard layout: DE");
       }
       if (buf[2] == 0x26) { // win 9 = special WASD (NM) mapping for LodeRunner
        lodeRunnerMapping = true;
        Serial.println("Keyboard layout: LodeRunner WASD (NM) special");
       }
       if (buf[2] == 0x27) { // win 0 = save default keyboard layout
         EEPROM.write(0, CurrentLayout.GetLayout());
         Serial.println("Keyboard layout saved!");
       }
      buf[2] = 0;
      }
      if ((buf[0] == 0x05) && (buf[2] == 0x4C)){ // reset
       KeyEncoder.IIreset();
       Serial.println("Reset!");
      }    
      keyCode = CurrentLayout.Decode_USB(buf);
      if (keyCode) {
        lastKeyPressed = keyCode;
        add2Buffer();
        if (lodeRunnerMapping) {
          if (keyCode == 0x57) keyCode = 0x49; // W
          if (keyCode == 0x41) keyCode = 0x4A; // A
          if (keyCode == 0x53) keyCode = 0x4B; // S
          if (keyCode == 0x44) keyCode = 0x4C; // D
          if (keyCode == 0x4E) keyCode = 0x55; // N
          if (keyCode == 0x4D) keyCode = 0x4F; // M
        }
        KeyEncoder.IIputchar((char)keyCode);
        Serial.print("USB keyCode: 0x");
        Serial.print(keyCode, HEX);
        Serial.println(" ");  
      } else lastKeyPressed = 0;
    }
  }
}

USB Usb;
USBHub     Hub(&Usb);
HIDSelector hidSelector(&Usb);

void setup() {
  KeyEncoder.Init();
  digitalWrite(A2LED, HIGH);

  Serial.begin(115200);
  Serial.println("");
  Serial.println("Welcome to:");
  Serial.println("");

  for (int i = 0; i < 11; i++) {
    strcpy_P(buffer, (char *)pgm_read_word(&(string_table[i]))); // Necessary casts and dereferencing, just copy.
    Serial.println(buffer);
  }
  
  for (int i = 0; i < MAX_REPEAT_BUFFER; i++) {
    repeatBuffer[i] = 0x0D;
  }

  Serial.println("");
  Serial.println("APPLE ][ USB dual Keyboard Adapter: Start-up");

  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");
  else
    digitalWrite(A2LED, LOW);

  // Set this to higher values to enable more debug information
  // minimum 0x00, maximum 0xff, default 0x80
  UsbDEBUGlvl = 0xff;

  CurrentLayout;
  CurrentLayout.ToggleCapsLock();
  if (EEPROM.read(0) > maxLayout) 
    CurrentLayout.SetLayout(DefaultKeyboardLayout);
  else
    CurrentLayout.SetLayout(EEPROM.read(0));
  Serial.print("Current keyboard layout is: ");
  if (CurrentLayout.GetLayout() == 1) 
    Serial.println("US");
  else 
    Serial.println("DE");

  delay(200);
  timer1.register_callback(timerCallback);
}

void loop() {
  timer1.run(REPEAT_INTERVAL_MS);
  Usb.Task();
}
