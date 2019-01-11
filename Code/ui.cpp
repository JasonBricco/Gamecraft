//
// Jason Bricco
//

static void SetClipboardText(void* data, const char* text)
{
    glfwSetClipboardString((GLFWwindow*)data, text);
}

static const char* GetClipboardText(void* data)
{
    return glfwGetClipboardString((GLFWwindow*)data);
}

static void InitUI(GLFWwindow* window, UI& ui)
{
	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "Gamecraft";
    io.IniFilename = NULL;

    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.SetClipboardTextFn = SetClipboardText;
    io.GetClipboardTextFn = GetClipboardText;
    io.ClipboardUserData = window;
    io.ImeWindowHandle = (void*)glfwGetWin32Window(window);

    ui.cursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    ui.cursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    ui.cursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); 
    ui.cursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    ui.cursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    ui.cursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    ui.cursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    ui.cursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

    // Create font texture.
    uint8_t* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    glGenTextures(1, &ui.fontTexture);
    glBindTexture(GL_TEXTURE_2D, ui.fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    io.Fonts->TexID = (ImTextureID)(intptr_t)ui.fontTexture;

    // Create VAO.
    glGenVertexArrays(1, &ui.va);
    glBindVertexArray(ui.va);

    glGenBuffers(1, &ui.vb);
    glBindBuffer(GL_ARRAY_BUFFER, ui.vb);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &ui.ib);
}

static void UI_UpdateMouse(GLFWwindow* window, UI& ui)
{
    ImGuiIO& io = ImGui::GetIO();

    for (int i = 0; i < 2; i++)
    {
        io.MouseDown[i] = ui.mouseJustPressed[i] || glfwGetMouseButton(window, i) != 0;
        ui.mouseJustPressed[i] = false;
    }

    ImVec2 mouseP = io.MousePos;
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

    bool focused = glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0;

    if (focused)
    {
        if (io.WantSetMousePos)
            glfwSetCursorPos(window, (double)mouseP.x, (double)mouseP.y);
        else
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
        }
    }

    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        return;

    ImGuiMouseCursor cursor = ImGui::GetMouseCursor();

    if (cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    else
    {
        glfwSetCursor(window, ui.cursors[cursor] ? ui.cursors[cursor] : ui.cursors[ImGuiMouseCursor_Arrow]);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

static void BeginNewUIFrame(GLFWwindow* window, UI& ui, float deltaTime)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built!");

    int w, h;
    glfwGetWindowSize(window, &w, &h);

    int displayW, displayH;
    glfwGetFramebufferSize(window, &displayW, &displayH);

    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)displayW / w) : 0, h > 0 ? ((float)displayH / h) : 0);

    io.DeltaTime = deltaTime;

    UI_UpdateMouse(window, ui);
    ImGui::NewFrame();
}

static void CreateUI(GLFWwindow* window)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 size = io.DisplaySize;

    if (g_paused)
    {
        ImVec2 panelSize = ImVec2(200.0f, 400.0f);

        ImGui::SetNextWindowSize(panelSize);
        ImGui::SetNextWindowPos(ImVec2(size.x * 0.5f - (panelSize.x * 0.5f), size.y * 0.5f - (panelSize.y * 0.5f)));

        ImGui::Begin("Paused");

        if (ImGui::Button("Continue"))
            Unpause(window);

        if (ImGui::Button("Quit"))
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        ImGui::End();
    }
}

static void RenderUI(GameState* state, Camera* cam, UI& ui)
{
    ImGui::Render();

    ImGuiIO& io = ImGui::GetIO();

    ImDrawData* data = ImGui::GetDrawData();

    int w = (int)(data->DisplaySize.x * io.DisplayFramebufferScale.x);
    int h = (int)(data->DisplaySize.y * io.DisplayFramebufferScale.y);

    if (w <= 0 || h <= 0)
        return;

    data->ScaleClipRects(io.DisplayFramebufferScale);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);

    Shader* shader = GetShader(state, SHADER_UI);
    UseShader(shader);
    SetUniform(shader->proj, cam->ortho);

    glBindVertexArray(ui.va);
    ImVec2 pos = data->DisplayPos;

    for (int n = 0; n < data->CmdListsCount; n++)
    {
        ImDrawList* cmdList = data->CmdLists[n];
        ImDrawIdx* bufferOffset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, ui.vb);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmdList->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmdList->VtxBuffer.Data, GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui.ib);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmdList->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmdList->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int i = 0; i < cmdList->CmdBuffer.Size; i++)
        {
            ImDrawCmd* cmd = &cmdList->CmdBuffer[i];

            if (cmd->UserCallback)
                cmd->UserCallback(cmdList, cmd);
            else
            {
                ImVec4 clip = ImVec4(cmd->ClipRect.x - pos.x, cmd->ClipRect.y - pos.y, cmd->ClipRect.z - pos.x, cmd->ClipRect.w - pos.y);

                // Assumes the clip origin is the lower-left corner of the screen.
                if (clip.x < w && clip.y < h && clip.z >= 0.0f && clip.w >= 0.0f)
                {
                    glScissor((int)clip.x, (int)(h - clip.w), (int)(clip.z - clip.x), (int)(clip.w - clip.y));
                    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)cmd->TextureId);
                    glDrawElements(GL_TRIANGLES, (GLsizei)cmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, bufferOffset);
                }
            }

            bufferOffset += cmd->ElemCount;
        }
    }

    glDisable(GL_SCISSOR_TEST);
}
