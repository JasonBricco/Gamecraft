//
// Gamecraft
//

enum KeyType
{
    KEY_UP,
    KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_SPACE,
    KEY_SHIFT, KEY_ESCAPE, KEY_TAB,
    KEY_P, KEY_T, KEY_R, KEY_E, KEY_C,
    KEY_BACKSPACE, KEY_SLASH, KEY_BACKSLASH,
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,
    KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4,
    KEY_MINUS, KEY_EQUAL, KEY_ENTER,
    KEY_COUNT
};
 
struct Input
{   
    bool mousePressed[2];
    bool mouseHeld[2];
    bool keys[KEY_COUNT];
    bool single[KEY_COUNT];
    float blockSetDelay;
};
