// Voxel Engine
// Jason Bricco

enum KeyType
{
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_SPACE,
    KEY_SHIFT,
    KEY_ESCAPE,
    KEY_TAB,
    KEY_P,
    KEY_T,
    KEY_BACKSPACE,
    KEY_COUNT
};
 
struct GameInput
{   
    bool mousePressed[2];
    bool mouseHeld[2];
    bool keys[KEY_COUNT];
    bool single[KEY_COUNT];
};
