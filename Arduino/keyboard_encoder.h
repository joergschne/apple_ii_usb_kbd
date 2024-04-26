#pragma once

//Arduino Pin definitioin for Keyboard Interface
#define IIKEYB_D0 A0
#define IIKEYB_D1 A1
#define IIKEYB_D2 A2
#define IIKEYB_D3 A3
#define IIKEYB_D4 A4
#define IIKEYB_D5 A5
#define IIKEYB_D6 2
#define IIKEYB_RESET 4
#define IIKEYB_STRB 3
#define A2LATCH 5
#define A2LED 6

class KeyEncoder
{
  public:
    void Init(void);
    void Disable(void);
    void IIreset(void);
    void IIstrobe(void);
    int IIputchar(int bOutput);
    int IIputs(const char *string);  
};

