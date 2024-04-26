/*! \file II_Encoder.cpp 
 * \brief Output ASCII in parallel form to Apple II keyboard connector.
 *
 *  Takes a ASCII character or string and sets the IO Pins accordingly for APPLE II keayboard Input.
 */

#include "Arduino.h"
#include "keyboard_encoder.h"

void KeyEncoder::Init(void)
{
  pinMode(IIKEYB_D0, OUTPUT);
  pinMode(IIKEYB_D1, OUTPUT);
  pinMode(IIKEYB_D2, OUTPUT);
  pinMode(IIKEYB_D3, OUTPUT);
  pinMode(IIKEYB_D4, OUTPUT);
  pinMode(IIKEYB_D5, OUTPUT);
  pinMode(IIKEYB_D6, OUTPUT);
  pinMode(IIKEYB_RESET, INPUT_PULLUP);
  pinMode(IIKEYB_STRB, OUTPUT);
  pinMode(A2LATCH, OUTPUT);
  pinMode(A2LED, OUTPUT);

  digitalWrite(IIKEYB_STRB, LOW);
  digitalWrite(A2LATCH, LOW);
  digitalWrite(A2LED, LOW);
}

void KeyEncoder::Disable(void)
{
  digitalWrite(IIKEYB_D0, LOW);
  digitalWrite(IIKEYB_D1, LOW);
  digitalWrite(IIKEYB_D2, LOW);
  digitalWrite(IIKEYB_D3, LOW);
  digitalWrite(IIKEYB_D4, LOW);
  digitalWrite(IIKEYB_D5, LOW);
  digitalWrite(IIKEYB_D6, LOW);
  digitalWrite(A2LATCH, LOW);
}

void KeyEncoder::IIreset(void)
{
  digitalWrite(A2LATCH, LOW);
  pinMode(IIKEYB_RESET, OUTPUT);
  digitalWrite(IIKEYB_RESET, LOW);
  delay(10);
  pinMode(IIKEYB_RESET, INPUT_PULLUP);
  digitalWrite(IIKEYB_RESET, HIGH);
}

void KeyEncoder::IIstrobe(void)
{
  digitalWrite(A2LED, HIGH);
  digitalWrite(A2LATCH, HIGH);
  digitalWrite(IIKEYB_STRB, HIGH);
  delay(5);
  digitalWrite(IIKEYB_STRB, LOW);    
  digitalWrite(A2LED, LOW);
}

int KeyEncoder::IIputchar(int bOutput)
{
  if (bOutput & 0b00000001)
    digitalWrite(IIKEYB_D0, HIGH);
  else
    digitalWrite(IIKEYB_D0, LOW);

  if (bOutput & 0b00000010)
    digitalWrite(IIKEYB_D1, HIGH);
  else
    digitalWrite(IIKEYB_D1, LOW);

  if (bOutput & 0b00000100)
    digitalWrite(IIKEYB_D2, HIGH);
  else
    digitalWrite(IIKEYB_D2, LOW);

  if (bOutput & 0b00001000)
    digitalWrite(IIKEYB_D3, HIGH);
  else
    digitalWrite(IIKEYB_D3, LOW);

  if (bOutput & 0b00010000)
    digitalWrite(IIKEYB_D4, HIGH);
  else
    digitalWrite(IIKEYB_D4, LOW);

  if (bOutput & 0b00100000)
    digitalWrite(IIKEYB_D5, HIGH);
  else
    digitalWrite(IIKEYB_D5, LOW);

  if (bOutput & 0b01000000)
    digitalWrite(IIKEYB_D6, HIGH);
  else
    digitalWrite(IIKEYB_D6, LOW);
  
  delay(5);
  IIstrobe();
  return (bOutput);
}

int KeyEncoder::IIputs(const char *string)
{
  int i = 0;
  while (string[i])
  {
    if (IIputchar(string[i]) == EOF)
    {
      return EOF;
    }
    i++;
  }

  if (IIputchar('\r') == EOF)
  {
    return EOF;
  }
  return 1;
}

