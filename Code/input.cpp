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

inline bool MouseHeld()
{
	return g_input.mouseDown;
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
}

static void OnKey(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	switch (key)
	{
		case GLFW_KEY_W:
			SetKey(KeyUp, action);
			break;

		case GLFW_KEY_S:
			SetKey(KeyDown, action);
			break;

		case GLFW_KEY_A:
			SetKey(KeyLeft, action);
			break;

		case GLFW_KEY_D:
			SetKey(KeyRight, action);
			break;

		case GLFW_KEY_SPACE:
			SetKey(KeySpace, action);
			break;

		case GLFW_KEY_ESCAPE:
			SetKey(KeyEscape, action);
			break;

		case GLFW_KEY_TAB:
			SetKey(KeyTab, action);
			break;
	}

	if (mode == GLFW_MOD_SHIFT) 
		SetKey(KeyShift, GLFW_PRESS);
	else SetKey(KeyShift, GLFW_RELEASE);
}
