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
    TrackGLAllocs(4);
}

static inline void MultiSpacing(int amount)
{
    for (int i = 0; i < amount; i++)
        ImGui::Spacing();
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

static inline ImTextureID GetUITexture(GameState* state, ImageID id)
{
    return (void*)(intptr_t)GetTexture(state, id).id;
}

static inline void BlockButton(World* world, GLFWwindow* window, GameState* state, ImageID image, BlockType type, char** name)
{
    if (ImGui::ImageButton(GetUITexture(state, image), ImVec2(32.0f, 32.0f)))
    {
        world->blockToSet = type;
        Unpause(state, window);
    }

    if (ImGui::IsItemHovered())
        *name = GetBlockName(world, type);
}

static ImVec2 CreateUIWindow(float width, float height)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    ImVec2 panelSize = ImVec2(width, height);
    ImGui::SetNextWindowSize(panelSize);
    ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f - (panelSize.x * 0.5f), displaySize.y * 0.5f - (panelSize.y * 0.5f)));

    return panelSize;
}

static void CreateBlockUI(GLFWwindow* window, World* world, GameState* state)
{
    ImVec2 size = CreateUIWindow(248.0f, 200.0f);

    char* blockName = NULL;

    ImGui::Begin("BlockSelect", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06666f, 0.06666f, 0.06666f, 1.0f));

    BlockButton(world, window, state, IMAGE_CRATE, BLOCK_CRATE, &blockName);
    ImGui::SameLine();
    BlockButton(world, window, state, IMAGE_DIRT, BLOCK_DIRT, &blockName);
    ImGui::SameLine();
    BlockButton(world, window, state, IMAGE_GRASS_SIDE, BLOCK_GRASS, &blockName);
    ImGui::SameLine();
    BlockButton(world, window, state, IMAGE_SAND, BLOCK_SAND, &blockName);
    ImGui::SameLine();
    BlockButton(world, window, state, IMAGE_STONE, BLOCK_STONE, &blockName);
    ImGui::Spacing();
    BlockButton(world, window, state, IMAGE_STONE_BRICK, BLOCK_STONE_BRICK, &blockName);
    ImGui::SameLine();
    BlockButton(world, window, state, IMAGE_METAL_CRATE, BLOCK_METAL_CRATE, &blockName);
    ImGui::SameLine();
    BlockButton(world, window, state, IMAGE_WATER, BLOCK_WATER, &blockName);
    ImGui::SameLine();
    BlockButton(world, window, state, IMAGE_WOOD_TOP, BLOCK_WOOD, &blockName);
    ImGui::SameLine();
    BlockButton(world, window, state, IMAGE_LEAVES, BLOCK_LEAVES, &blockName);
    ImGui::Spacing();
    BlockButton(world, window, state, IMAGE_CLAY, BLOCK_CLAY, &blockName);
    ImGui::SameLine();
    BlockButton(world, window, state, IMAGE_SNOW_SIDE, BLOCK_SNOW, &blockName);
 
    ImGui::PopStyleColor();

    if (blockName != NULL)
    {
        ImVec2 cursor = ImGui::GetCursorPos();
        ImVec2 textSize = ImGui::CalcTextSize(blockName);
        ImVec2 cursorPos = ImVec2(size.x * 0.5f - (textSize.x * 0.5f), size.y - 25.0f);

        ImGui::SetCursorPos(cursorPos);
        ImGui::Text(blockName);
        ImGui::SetCursorPos(cursor);
    }

    ImGui::End();
}

static void CreatePauseUI(GameState* state, GLFWwindow* window)
{
    ImVec2 size = CreateUIWindow(175.0f, 220.0f);

    ImGui::Begin("Pause", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav);

    ImVec2 textSize = ImGui::CalcTextSize("Paused");

    ImVec2 cursorPos = ImVec2(size.x * 0.5f - (textSize.x * 0.5f), 10.0f);
    ImGui::SetCursorPos(cursorPos);

    ImGui::Text("Paused");

    ImVec2 btnSize = ImVec2(100.0f, 30.0f);
    cursorPos = ImVec2(size.x * 0.5f - (btnSize.x * 0.5f), 35.0f);

    ImGui::SetCursorPos(cursorPos);

    if (ImGui::Button("Continue", btnSize))
        Unpause(state, window);

    cursorPos.y += 35.0f;
    ImGui::SetCursorPos(cursorPos);

    if (ImGui::Button("New Island", btnSize))
        state->pauseState = WORLD_CONFIG;

    cursorPos.y += 35.0f;
    ImGui::SetCursorPos(cursorPos);

    bool raining = state->rain.active;

    if (ImGui::Button(raining ? "Disable Rain" : "Enable Rain", btnSize))
    {
        state->rain.active = !raining;
        Unpause(state, window);
    }

    cursorPos.y += 35.0f;
    ImGui::SetCursorPos(cursorPos);

    bool muted = state->audio.muted;

    if (ImGui::Button(muted ? "Unmute Audio" : "Mute Audio", btnSize))
        ToggleMute(&state->audio);

    cursorPos.y += 35.0f;
    ImGui::SetCursorPos(cursorPos);

    if (ImGui::Button("Quit", btnSize))
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    ImGui::End();
}

static void WorldConfigUI(GLFWwindow* window, GameState* state, World* world, WorldConfig& config, Player* player)
{
    ImVec2 size = CreateUIWindow(300.0f, 135.0f);

    ImGui::Begin("WorldConfig", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav);

    char* text = "World Settings";
    ImVec2 textSize = ImGui::CalcTextSize(text);

    ImVec2 cursorPos = ImVec2(size.x * 0.5f - (textSize.x * 0.5f), 10.0f);
    ImGui::SetCursorPos(cursorPos);

    ImGui::Text("World Settings");

    MultiSpacing(3);

    ImGui::Text("Radius");
    ImGui::SameLine();

    if (config.infinite)
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

    ImGui::PushItemWidth(130.0f);
    ImGui::InputText("##hidelabel", config.radiusBuffer, sizeof(config.radiusBuffer), ImGuiInputTextFlags_CharsDecimal);
    ImGui::PopItemWidth();

    if (config.infinite)
        ImGui::PopStyleVar();

    ImGui::SameLine();
    ImGui::Checkbox("Infinite", &config.infinite);

    bool biomes[BIOME_COUNT] = {};
    biomes[config.biome] = true;

    MultiSpacing(2);
    ImGui::Text("Biome");
    ImGui::SameLine();

    for (int i = 0; i < BIOME_COUNT; i++)
    {
        Biome& biome = world->biomes[i];

        if (ImGui::Checkbox(biome.name, &biomes[i]))
            config.biome = (BiomeType)i;

        ImGui::SameLine();
    }

    ImVec2 btnSize = ImVec2(100.0f, 25.0f);
    ImGui::SetCursorPos(ImVec2(size.x * 0.5f - (btnSize.x * 0.5f), size.y - 35.0f));

    if (ImGui::Button("Generate", ImVec2(100.0f, 25.0f)))
    {
        int radius = atoi(config.radiusBuffer);
        memset(config.radiusBuffer, 0, sizeof(config.radiusBuffer));

        if (radius >= 32 || config.infinite)
        {
            config.radius = radius;
            RegenerateWorld(state, world, config);
            player->spawned = false;
            player->velocity.y = 0.0f;
            Unpause(state, window);
        }
    }

    ImGui::End();
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

    float L = data->DisplayPos.x;
    float R = data->DisplayPos.x + data->DisplaySize.x;
    float T = data->DisplayPos.y;
    float B = data->DisplayPos.y + data->DisplaySize.y;

    mat4 proj = ortho(L, R, B, T);

    if (state->pauseState == PLAYING)
    {
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
        Graphic* crosshair = cam->crosshair;
        Shader* shader = crosshair->shader;
        UseShader(shader);
        SetUniform(shader->proj, proj);
        DrawGraphic(crosshair, shader);
    }

    Shader* shader = GetShader(state, SHADER_UI);
    UseShader(shader);
    SetUniform(shader->proj, proj);

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
