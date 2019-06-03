//
// Gamecraft
//

static inline bool KeyHeld(Input& input, KeyType type)
{
	return input.keys[type];
}

static inline bool KeyPressed(Input& input, KeyType type)
{
	return input.single[type];
}

static inline bool MousePressed(Input& input, int button)
{
	return input.mousePressed[button];
}

static inline bool MouseHeld(Input& input, int button)
{
	return input.mouseHeld[button];
}

static void SetKey(Input& input, KeyType type, int action)
{
	if (action == GLFW_RELEASE)
		input.keys[type] = false;
	else
	{
		if (!input.keys[type])
			input.single[type] = true;
		
		input.keys[type] = true;
	}		
}

inline void ResetInput(Input& input)
{
	memset(&input.single, 0, sizeof(input.single));
	memset(&input.mousePressed, 0, sizeof(input.mousePressed));
}

static void InputCharCallback(GLFWwindow*, unsigned int c)
{
    ImGuiIO& io = ImGui::GetIO();

    if (c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

static void OnKey(GLFWwindow* window, int key, int, int action, int mode)
{
	Input& input = ((GameState*)glfwGetWindowUserPointer(window))->input;

	ImGuiIO& io = ImGui::GetIO();
   
	if (action == GLFW_PRESS)
		io.KeysDown[key] = true;

	if (action == GLFW_RELEASE)
		io.KeysDown[key] = false;

	switch (key)
	{
		case GLFW_KEY_W:
			SetKey(input, KEY_UP, action);
			break;

		case GLFW_KEY_S:
			SetKey(input, KEY_DOWN, action);
			break;

		case GLFW_KEY_A:
			SetKey(input, KEY_LEFT, action);
			break;

		case GLFW_KEY_D:
			SetKey(input, KEY_RIGHT, action);
			break;

		case GLFW_KEY_C:
			SetKey(input, KEY_C, action);
			break;

		case GLFW_KEY_SPACE:
			SetKey(input, KEY_SPACE, action);
			break;

		case GLFW_KEY_ESCAPE:
			SetKey(input, KEY_ESCAPE, action);
			break;

		case GLFW_KEY_TAB:
			SetKey(input, KEY_TAB, action);
			break;

		case GLFW_KEY_P:
			SetKey(input, KEY_P, action);
			break;

		case GLFW_KEY_T:
			SetKey(input, KEY_T, action);
			break;

		case GLFW_KEY_R:
			SetKey(input, KEY_R, action);
			break;

		case GLFW_KEY_E:
			SetKey(input, KEY_E, action);
			break;

		case GLFW_KEY_BACKSPACE:
			SetKey(input, KEY_BACKSPACE, action);
			break;

		case GLFW_KEY_SLASH:
			SetKey(input, KEY_SLASH, action);
			break;
			
		case GLFW_KEY_BACKSLASH:
			SetKey(input, KEY_BACKSLASH, action);
			break;

		case GLFW_KEY_ENTER:
			SetKey(input, KEY_ENTER, action);
			break;

		case GLFW_KEY_0:
			SetKey(input, KEY_0, action);
			break;

		case GLFW_KEY_1:
			SetKey(input, KEY_1, action);
			break;

		case GLFW_KEY_2:
			SetKey(input, KEY_2, action);
			break;

		case GLFW_KEY_3:
			SetKey(input, KEY_3, action);
			break;

		case GLFW_KEY_4:
			SetKey(input, KEY_4, action);
			break;

		case GLFW_KEY_5:
			SetKey(input, KEY_5, action);
			break;

		case GLFW_KEY_6:
			SetKey(input, KEY_6, action);
			break;

		case GLFW_KEY_7:
			SetKey(input, KEY_7, action);
			break;

		case GLFW_KEY_8:
			SetKey(input, KEY_8, action);
			break;

		case GLFW_KEY_9:
			SetKey(input, KEY_9, action);
			break;

		case GLFW_KEY_MINUS:
			SetKey(input, KEY_MINUS, action);
			break;

		case GLFW_KEY_EQUAL:
			SetKey(input, KEY_EQUAL, action);
			break;

		case GLFW_KEY_F1:
			SetKey(input, KEY_F1, action);
			break;

		case GLFW_KEY_F2:
			SetKey(input, KEY_F2, action);
			break;

		case GLFW_KEY_F3:
			SetKey(input, KEY_F3, action);
			break;

		case GLFW_KEY_F4:
			SetKey(input, KEY_F4, action);
			break;
	}

	if (mode == GLFW_MOD_SHIFT) 
		SetKey(input, KEY_SHIFT, GLFW_PRESS);
	else SetKey(input, KEY_SHIFT, GLFW_RELEASE);
}

static void OnMouseButton(GLFWwindow* window, int button, int action, int)
{
	int i = -1;

	if (button == GLFW_MOUSE_BUTTON_LEFT) i = 0;
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) i = 1;

	if (i >= 0)
	{
		Input& input = ((GameState*)glfwGetWindowUserPointer(window))->input;
		
		switch (action)
		{
			case GLFW_PRESS:
				input.mousePressed[i] = true;
				input.mouseHeld[i] = true;
		    	break;

		    case GLFW_RELEASE:
		    	input.mouseHeld[i] = false;
		}
	}
}
