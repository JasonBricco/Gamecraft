// Voxel Engine
// Jason Bricco

enum KeyType
{
    KeyUp,
    KeyDown,
    KeyLeft,
    KeyRight,
    KeySpace,
    KeyShift,
    KeyEscape,
    KeyTab,
    KeyCount
};
 
struct GameInput
{   
    bool mouseDown;
    bool keys[KeyCount];
    bool single[KeyCount];
};
 
inline bool KeyHeld(KeyType type);
inline bool KeyPressed(KeyType type);
inline bool MouseHeld();
static void SetKey(KeyType type, int action);
inline void ResetInput();

static void OnKey(GLFWwindow* window, int key, int scancode, int action, int mode);
