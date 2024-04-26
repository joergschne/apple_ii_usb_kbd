 #pragma once
 
class KeyDecoder
{
        static const uint8_t numKeys1[10];
        static const uint8_t numKeysAlt1[10];
        static const uint8_t symKeysUp1[12];
        static const uint8_t symKeysLo1[12];

        static const uint8_t numKeys2[10];
        static const uint8_t numKeysAlt2[10];
        static const uint8_t symKeysUp2[12];
        static const uint8_t symKeysLo2[12];
        static uint8_t lastBuffer[21];
        static uint8_t deltaK;
        static uint8_t pos;
//		uint8_t uiCurrectSelectedLayout;
  public:
    KeyDecoder();
    void SetLayout(uint8_t layout);
    uint8_t GetLayout(void);
    void ToggleCapsLock(void);
    uint8_t Decode_NKRO(uint8_t *buf);
    uint8_t Decode_USB(uint8_t *buf);

  private:
    uint8_t currentLayout;//ID of the current active keyboard Layout
    boolean capsLock;
};

