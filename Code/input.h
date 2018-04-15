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
    KEY_COUNT
};
 
struct GameInput
{   
    bool mousePressed[2];
    bool mouseHeld[2];
    bool keys[KEY_COUNT];
    bool single[KEY_COUNT];
};
 
inline bool KeyHeld(KeyType type);
inline bool KeyPressed(KeyType type);

inline bool MousePressed(int button);
inline bool MouseHeld(int button);

static void SetKey(KeyType type, int action);
inline void ResetInput();

static void OnKey(GLFWwindow* window, int key, int scancode, int action, int mode);
static void OnMouseButton(GLFWwindow* window, int button, int action, int mods);
