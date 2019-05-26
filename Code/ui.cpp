//
// Gamecraft
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

    io.DisplaySize = ImVec2((float)displayW, (float)displayH);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)displayW / w) : 0, h > 0 ? ((float)displayH / h) : 0);

    io.DeltaTime = deltaTime;

    UI_UpdateMouse(window, ui);
    ImGui::NewFrame();
}

static inline ImTextureID GetUITexture(GameState* state, ImageID id)
{
    return (void*)(intptr_t)GetTexture(state, id).id;
}

static inline void BlockButton(World* world, GameState* state, ImageID image, BlockType type, char** name)
{
    if (ImGui::ImageButton(GetUITexture(state, image), ImVec2(32.0f, 32.0f)))
    {
        world->blockToSet = type;
        Unpause(state);
    }

    if (ImGui::IsItemHovered())
        *name = GetBlockName(world, type);
}

static ImVec2 UIWindow(float width, float height, ImVec2 pos)
{
    ImVec2 panelSize = ImVec2(width, height);
    ImGui::SetNextWindowSize(panelSize);
    ImGui::SetNextWindowPos(pos);
    return panelSize;
}

static ImVec2 CenteredUIWindow(float width, float height, float offsetX = 0.0f, float offsetY = 0.0f)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    ImVec2 p = ImVec2(displaySize.x * 0.5f - (width * 0.5f), displaySize.y * 0.5f - (height * 0.5f));
    p.x += offsetX;
    p.y += offsetY;
    return UIWindow(width, height, p);
}

static void CreateBlockUI(World* world, GameState* state)
{
    ImVec2 size = CenteredUIWindow(248.0f, 200.0f);

    char* blockName = NULL;

    ImGui::Begin("BlockSelect", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06666f, 0.06666f, 0.06666f, 1.0f));

    BlockButton(world, state, IMAGE_CRATE, BLOCK_CRATE, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_DIRT, BLOCK_DIRT, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_GRASS_SIDE, BLOCK_GRASS, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_SAND, BLOCK_SAND, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_STONE, BLOCK_STONE, &blockName);
    ImGui::Spacing();
    BlockButton(world, state, IMAGE_STONE_BRICK, BLOCK_STONE_BRICK, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_METAL_CRATE, BLOCK_METAL_CRATE, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_WATER, BLOCK_WATER, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_WOOD, BLOCK_WOOD, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_LEAVES, BLOCK_LEAVES, &blockName);
    ImGui::Spacing();
    BlockButton(world, state, IMAGE_CLAY, BLOCK_CLAY, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_SNOW_SIDE, BLOCK_SNOW, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_ICE, BLOCK_ICE, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_LANTERN_ON, BLOCK_LANTERN, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_TRAMPOLINE, BLOCK_TRAMPOLINE, &blockName);
    ImGui::Spacing();
    BlockButton(world, state, IMAGE_CACTUS_SIDE, BLOCK_CACTUS, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_MAGMA, BLOCK_MAGMA, &blockName);

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

static inline void WindowHeader(char* text, float windowWidth)
{
    ImVec2 textSize = ImGui::CalcTextSize(text);

    ImVec2 cursorPos = ImVec2(windowWidth * 0.5f - (textSize.x * 0.5f), 10.0f);
    ImGui::SetCursorPos(cursorPos);

    ImGui::Text(text);
}

static inline ImVec2 GetButtonLoc(ImVec2 btnSize, float winW, float y)
{
    return ImVec2(winW * 0.5f - (btnSize.x * 0.5f), y);
}

static void CreatePauseUI(GameState* state, World* world, GLFWwindow* window)
{
    ImVec2 size = CenteredUIWindow(175.0f, 255.0f);

    ImGui::Begin("Pause", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav);
    WindowHeader("Paused", size.x);

    ImVec2 btnSize = ImVec2(110.0f, 30.0f);
    ImVec2 cursorPos = GetButtonLoc(btnSize, size.x, 35.0f);

    ImGui::SetCursorPos(cursorPos);

    if (ImGui::Button("Continue", btnSize))
        Unpause(state);

    cursorPos.y += 35.0f;
    ImGui::SetCursorPos(cursorPos);

    if (ImGui::Button("New Island", btnSize))
        state->pauseState = WORLD_CONFIG;

    cursorPos.y += 35.0f;
    ImGui::SetCursorPos(cursorPos);

    Biome& biome = GetCurrentBiome(world);

    Weather& weather = biome.weather;
    bool weatherOn = weather.emitter->active;

    if (ImGui::Button(weatherOn ? "Disable Weather" : "Enable Weather", btnSize))
    {
        weather.emitter->active = !weatherOn;

        if (!weatherOn)
            FadeAmbient(world, state->renderer, 0.6f, 3.0f);
        else FadeAmbient(world, state->renderer, 1.0f, 3.0f);

        Unpause(state);
    }

    cursorPos.y += 35.0f;
    ImGui::SetCursorPos(cursorPos);

    bool isNight = state->renderer.ambient < 0.2f;

    if (ImGui::Button(isNight ? "Set Day" : "Set Night", btnSize))
    {
        if (isNight)
            FadeAmbient(world, state->renderer, 1.0f, 0.5f);
        else FadeAmbient(world, state->renderer, 0.05f, 0.5f);

        Unpause(state);
    }

    cursorPos.y += 35.0f;
    ImGui::SetCursorPos(cursorPos);

    if (ImGui::Button("Settings", btnSize))
        state->pauseState = GAME_SETTINGS;

    cursorPos.y += 35.0f;
    ImGui::SetCursorPos(cursorPos);

    if (ImGui::Button("Quit", btnSize))
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    ImGui::End();
}

static void CreateSettingsUI(GameState* state)
{
    Renderer& rend = state->renderer;
    ImVec2 size = CenteredUIWindow(200.0f, 150.0f);

    ImGui::Begin("Settings", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav);
    WindowHeader("Settings", size.x);

    ImVec2 btnSize = ImVec2(125.0f, 30.0f);
    ImVec2 cursorPos = GetButtonLoc(btnSize, size.x, 35.0f);
    ImGui::SetCursorPos(cursorPos);

    bool muted = state->audio.muted;

    if (ImGui::Button(muted ? "Unmute Audio" : "Mute Audio", btnSize))
        ToggleMute(&state->audio);

    cursorPos.y += 35.0f;
    ImGui::SetCursorPos(cursorPos);

    bool aaOn = rend.samplesAA == 4;

    if (ImGui::Button(aaOn ? "Antialiasing On" : "Antialiasing Off", btnSize))
        SetAA(rend, aaOn ? 0 : 4);

    cursorPos.y += 35.0f;
    ImGui::SetCursorPos(cursorPos);

    if (ImGui::Button("Go Back", btnSize))
        state->pauseState = PAUSED;

    ImGui::End();
}

static void WorldConfigUI(GameState* state, World* world, WorldConfig& config)
{
    if (config.errorTime > 0.0f)
    {   
        config.errorTime -= state->deltaTime;
        ImVec2 textSize = ImGui::CalcTextSize(config.error);
        CenteredUIWindow(textSize.x + 15.0f, textSize.y, 0.0f, -115.0f);
        ImGui::Begin("WorldConfigError", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav);
        ImGui::Text(config.error);
        ImGui::End();
    }

    ImVec2 size = CenteredUIWindow(300.0f, 180.0f);

    ImGui::Begin("WorldConfig", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav);
    WindowHeader("World Settings", size.x);

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
    ImVec2 btSize = ImGui::CalcTextSize("Biome ");
    ImGui::SameLine();

    int numPerLine[] = { 3, 3, 1 };
    int curBiome = 0;

    ImVec2 baseCursor = ImGui::GetCursorPos();
    float biomeGap = 75.0f;

    for (int n = 0; n < ArrayLength(numPerLine); n++)
    {
        for (int i = 0; i < numPerLine[n]; i++)
        {
            Biome& biome = world->biomes[curBiome];

            if (ImGui::Checkbox(biome.name, &biomes[curBiome]))
                config.biome = (BiomeType)curBiome;

            curBiome++;
            ImGui::SameLine();
            ImGui::SetCursorPos(ImVec2(baseCursor.x + ((i + 1) * biomeGap), baseCursor.y));
        }

        ImGui::NewLine();
        baseCursor = ImGui::GetCursorPos();
        baseCursor.x += btSize.x + 1;
        ImGui::SetCursorPos(baseCursor);
    }

    ImVec2 btnSize = ImVec2(85.0f, 25.0f);
    ImVec2 loc = GetButtonLoc(btnSize, size.x, size.y - 35.0f);
    ImGui::SetCursorPos(ImVec2(loc.x - (btnSize.x * 0.5f) - 3.0f, loc.y));

    if (ImGui::Button("Generate", btnSize))
    {
        int radius = atoi(config.radiusBuffer);
        memset(config.radiusBuffer, 0, sizeof(config.radiusBuffer));

        if (radius < 32 && !config.infinite)
        {
            config.error = "Island radius must be 32+ or infinite.";
            config.errorTime = 3.0f;
        }
        else
        {
            config.radius = radius;
            state->pendingConfig = &config;
            Unpause(state);
            BeginLoadingScreen(state, 0.5f, RegenerateWorldCallback);
        }
    }

    ImGui::SetCursorPos(ImVec2(loc.x + (btnSize.x * 0.5f) + 3.0f, loc.y));

    if (ImGui::Button("Go Back", btnSize))
        state->pauseState = PAUSED;

    ImGui::End();
}

static double GetFPS()
{
    static double prevSec = 0.0;
    static int frameCount = 0;
    static double fps = 0.0;

    double current = glfwGetTime();
    double elapsed = current - prevSec;

    if (elapsed > 0.25)
    {
        prevSec = current;
        fps = (double)frameCount / elapsed;
        frameCount = 0;
    }

    frameCount++;
    return fps;
}

static void CreateProfilerUI()
{
    ivec2 size = FramebufferSize();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2((float)size.x, (float)size.y));

    ImGui::Begin("Profiler", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
    
    ImGui::Text("Profiler");
    ImGui::Spacing();

    DebugState& state = g_debugState;

    if (state.framesUntilUpdate <= 0)
    {
        UpdateDebugStats();
        state.framesUntilUpdate = MAX_DEBUG_SNAPSHOTS;
    }

    for (int i = 0; i < MAX_DEBUG_RECORDS; i++)
    {
        DebugCounters& c = state.counters[i];

        if (c.func == nullptr)
            continue;

        DebugStat& calls = state.calls[i];
        DebugStat& cycles = state.cycles[i];

        char buffer[128];
        sprintf(buffer, "%s (%i) - Avg Cycles: %llu, Avg Calls: %llu\n", c.func, c.line, cycles.avg, calls.avg);

        ImGui::Text(buffer);

        vector<float> histValues;
        histValues.reserve(MAX_DEBUG_SNAPSHOTS);

        for (int j = 0; j < MAX_DEBUG_SNAPSHOTS; j++)
            histValues.push_back((float)c.snapshots[j].cycles);

        ImGui::SameLine();

        ImGui::PushID(i);
        ImGui::PlotHistogram("", histValues.data(), (int)histValues.size(), 0, NULL, (float)cycles.min, (float)cycles.max, ImVec2(256.0f, 20.0f));
        ImGui::PopID();
    }

    ImGui::End();
}

static void CreateDebugHUD(World* world)
{
    TIMED_BLOCK;

    double fps = GetFPS();

    ivec2 size = FramebufferSize();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2((float)size.x, (float)size.y));

    ImGui::Begin("HUD", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);

    char fpsText[16];
    sprintf(fpsText, "FPS: %.1f", fps);

    ImGui::Text(fpsText);

    char buildText[32];
    sprintf(buildText, "Build: %i, Mode: %s", g_buildID, g_buildType);

    ImGui::Text(buildText);

    Biome& biome = GetCurrentBiome(world);

    char biomeText[32];
    sprintf(biomeText, "Biome: %s", biome.name);
    ImGui::Text(biomeText);

    WorldP playerBlock = LWorldToWorldP(world, BlockPos(world->player->pos));

    char playerP[64];
    sprintf(playerP, "Player Position: %i, %i, %i", playerBlock.x, playerBlock.y, playerBlock.z);

    ImGui::Text(playerP);
    
    if (world->cursorOnBlock)
    {
        Block block = GetBlock(world, world->cursorBlockPos);
        char* name = GetBlockName(world, block);

        ivec3 lwP = world->cursorBlockPos;
        WorldP p = LWorldToWorldP(world, lwP);

        char blockText[128];
        sprintf_s(blockText, 64, "Block: %s, Position: %i, %i, %i (%i, %i, %i)", name, lwP.x, lwP.y, lwP.z, p.x, p.y, p.z);

        ImGui::Text(blockText);
    }

    ImGui::End();
}

static void RenderUI(GameState* state, Renderer& rend, UI& ui)
{
    TIMED_BLOCK;

    ImGui::Render();

    ImGuiIO& io = ImGui::GetIO();
    ImDrawData* data = ImGui::GetDrawData();

    int w = (int)(data->DisplaySize.x * io.DisplayFramebufferScale.x);
    int h = (int)(data->DisplaySize.y * io.DisplayFramebufferScale.y);

    if (w <= 0 || h <= 0)
        return;

    data->ScaleClipRects(io.DisplayFramebufferScale);

    float L = data->DisplayPos.x;
    float R = data->DisplayPos.x + data->DisplaySize.x;
    float T = data->DisplayPos.y;
    float B = data->DisplayPos.y + data->DisplaySize.y;

    mat4 proj = ortho(L, R, B, T);

    if (state->pauseState == PLAYING)
    {
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
        Graphic* crosshair = rend.crosshair;
        Shader* shader = crosshair->shader;
        UseShader(shader);
        SetUniform(shader->proj, proj);
        DrawGraphic(crosshair, shader);
    }

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);

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

static void CreateUI(GameState* state, GLFWwindow* window, World* world, WorldConfig& config)
{
    switch (state->pauseState)
    {
        case PAUSED:
            CreatePauseUI(state, world, window);
            break;

        case SELECTING_BLOCK:
            CreateBlockUI(world, state);
            break;

        case GAME_SETTINGS:
            CreateSettingsUI(state);
            break;

        case WORLD_CONFIG:
            WorldConfigUI(state, world, config);
            break;
    }

    switch (state->debugDisplay)
    {
        case DEBUG_DISPLAY_HUD:
            CreateDebugHUD(world);
            break;

        case DEBUG_DISPLAY_PROFILER:
            CreateProfilerUI();
            break;
    }
}

static void SaveGameSettings(GameState* state)
{
    char path[MAX_PATH];
    sprintf(path, "%s\\Settings.txt", state->savePath);

    SettingsFileData settings = { state->audio.muted, state->renderer.samplesAA };
    WriteBinary(path, (char*)&settings, sizeof(SettingsFileData));
}

static void LoadGameSettings(GameState* state)
{
    char path[MAX_PATH];
    sprintf(path, "%s\\Settings.txt", state->savePath);

    Renderer& rend = state->renderer;
    
    if (PathFileExists(path))
    {
        SettingsFileData settings;
        ReadBinary(path, (char*)&settings);
        state->audio.muted = settings.audioMuted;
        rend.samplesAA = settings.samplesAA;
        SetAA(rend, settings.samplesAA);
    }
    else SetAA(rend, 4);
}
