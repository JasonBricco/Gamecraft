// Voxel Engine
// Jason Bricco

static GameInput g_input;

static inline bool KeyHeld(KeyType type)
{
	return g_input.keys[type];
}

static inline bool KeyPressed(KeyType type)
{
	return g_input.single[type];
}

static inline int LastNumKey()
{
	return g_input.lastNum;
}

static inline bool MousePressed(int button)
{
	return g_input.mousePressed[button];
}

static inline bool MouseHeld(int button)
{
	return g_input.mouseHeld[button];
}

static void SetKey(KeyType type, int action)
{
	if (action == GLFW_RELEASE)
		g_input.keys[type] = false;
	else
	{
		if (!g_input.keys[type])
			g_input.single[type] = true;
		
		g_input.keys[type] = true;
	}		
}

inline void ResetInput()
{
	memset(&g_input.single, 0, sizeof(g_input.single));
	memset(&g_input.mousePressed, 0, sizeof(g_input.mousePressed));
}

static void OnKey(GLFWwindow*, int key, int, int action, int mode)
{
	switch (key)
	{
		case GLFW_KEY_W:
			SetKey(KEY_UP, action);
			break;

		case GLFW_KEY_S:
			SetKey(KEY_DOWN, action);
			break;

		case GLFW_KEY_A:
			SetKey(KEY_LEFT, action);
			break;

		case GLFW_KEY_D:
			SetKey(KEY_RIGHT, action);
			break;

		case GLFW_KEY_SPACE:
			SetKey(KEY_SPACE, action);
			break;

		case GLFW_KEY_ESCAPE:
			SetKey(KEY_ESCAPE, action);
			break;

		case GLFW_KEY_TAB:
			SetKey(KEY_TAB, action);
			break;

		case GLFW_KEY_P:
			SetKey(KEY_P, action);
			break;

		case GLFW_KEY_T:
			SetKey(KEY_T, action);
			break;

		case GLFW_KEY_BACKSPACE:
			SetKey(KEY_BACKSPACE, action);
			break;

		case GLFW_KEY_BACKSLASH:
			SetKey(KEY_BACKSLASH, action);
			break;

		case GLFW_KEY_0:
			g_input.lastNum = 0;
			SetKey(KEY_0, action);
			break;

		case GLFW_KEY_1:
			g_input.lastNum = 1;
			SetKey(KEY_1, action);
			break;

		case GLFW_KEY_2:
			g_input.lastNum = 2;
			SetKey(KEY_2, action);
			break;

		case GLFW_KEY_3:
			g_input.lastNum = 3;
			SetKey(KEY_3, action);
			break;

		case GLFW_KEY_4:
			g_input.lastNum = 4;
			SetKey(KEY_4, action);
			break;

		case GLFW_KEY_5:
			g_input.lastNum = 5;
			SetKey(KEY_5, action);
			break;

		case GLFW_KEY_6:
			g_input.lastNum = 6;
			SetKey(KEY_6, action);
			break;

		case GLFW_KEY_7:
			g_input.lastNum = 7;
			SetKey(KEY_7, action);
			break;

		case GLFW_KEY_8:
			g_input.lastNum = 8;
			SetKey(KEY_8, action);
			break;

		case GLFW_KEY_9:
			g_input.lastNum = 9;
			SetKey(KEY_9, action);
			break;

		case GLFW_KEY_F1:
			SetKey(KEY_F1, action);
			break;
	}

	if (mode == GLFW_MOD_SHIFT) 
		SetKey(KEY_SHIFT, GLFW_PRESS);
	else SetKey(KEY_SHIFT, GLFW_RELEASE);
}

static void OnMouseButton(GLFWwindow*, int button, int action, int)
{
	int i = -1;

	if (button == GLFW_MOUSE_BUTTON_LEFT) i = 0;
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) i = 1;

	if (i >= 0)
	{
		switch (action)
		{
			case GLFW_PRESS:
				g_input.mousePressed[i] = true;
				g_input.mouseHeld[i] = true;
		    	break;

		    case GLFW_RELEASE:
		    	g_input.mouseHeld[i] = false;
		}
	}
}

#define g_input Error_Invalid_Use
