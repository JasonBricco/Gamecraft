// Voxel Engine
// Jason Bricco

static GameInput g_input;

inline bool KeyHeld(KeyType type)
{
	return g_input.keys[type];
}

inline bool KeyPressed(KeyType type)
{
	return g_input.single[type];
}

inline bool MousePressed()
{
	return g_input.mousePressed;
}

inline bool MouseHeld()
{
	return g_input.mouseHeld;
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
	g_input.mousePressed = false;
}

static void OnKey(GLFWwindow* window, int key, int scancode, int action, int mode)
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
	}

	if (mode == GLFW_MOD_SHIFT) 
		SetKey(KEY_SHIFT, GLFW_PRESS);
	else SetKey(KEY_SHIFT, GLFW_RELEASE);
}

static void OnMouseButton(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		switch (action)
		{
			case GLFW_PRESS:
		    	g_input.mousePressed = true;
		    	g_input.mouseHeld = true;
		    	break;

		    case GLFW_RELEASE:
		    	g_input.mouseHeld = false;
		}
	}
}
