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
        *name = GetName(world, type);
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
    ImVec2 size = CenteredUIWindow(248.0f, 210.0f);

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
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_COOLED_MAGMA, BLOCK_COOLED_MAGMA, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_OBSIDIAN, BLOCK_OBSIDIAN, &blockName);
    ImGui::SameLine();
    BlockButton(world, state, IMAGE_LAVA, BLOCK_LAVA, &blockName);

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
    float windowOffset = 0.0f;

    #if DEBUG_SERVICES
    windowOffset = -90.0f;
    #endif

    ImVec2 size = CenteredUIWindow(175.0f, 255.0f, windowOffset);

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

    #if DEBUG_SERVICES

    DebugTable& t = g_debugTable;

    size = CenteredUIWindow(175.0f, 255.0f, 90.0f);

    ImGui::Begin("DebugPause", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav);
    WindowHeader("Debug Menu", size.x);

    btnSize = ImVec2(140.0f, 30.0f);
    cursorPos = GetButtonLoc(btnSize, size.x, 35.0f);
    ImGui::SetCursorPos(cursorPos);

    bool recording = t.profilerState >= PROFILER_RECORDING;
    char* profileText = recording ? "Stop Profiling" : "Start Profiling";

    if (ImGui::Button(profileText, btnSize))
    {
        if (recording)
        {
            state->savedInputMode = glfwGetInputMode(window, GLFW_CURSOR);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            t.profilerState = PROFILER_STOPPED;
            state->pauseState = PLAYING;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, state->savedInputMode);
            CenterCursor();
            t.profilerState = PROFILER_RECORDING;
            Unpause(state);
        }
    }

    cursorPos.y += 35.0f;
    ImGui::SetCursorPos(cursorPos);

    char* outlineText = t.showOutlines ? "Hide Chunk Outlines" : "Show Chunk Outlines";

    if (ImGui::Button(outlineText, btnSize))
    {
        t.showOutlines = !t.showOutlines;
        Unpause(state);
    }

    ImGui::End();

    #endif
}

static void CreateDeathUI(GameState* state, World* world, GLFWwindow* window)
{
    ImVec2 size = CenteredUIWindow(300.0f, 255.0f);

    ImGui::Begin("Death", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav);
    WindowHeader("You died!", size.x);

    ImVec2 btnSize = ImVec2(110.0f, 30.0f);

    ImVec2 cursorPos = GetButtonLoc(btnSize, size.x, 35.0f);
    ImGui::SetCursorPos(ImVec2(cursorPos.x - (btnSize.x / 2) - 2, cursorPos.y));

    if (ImGui::Button("Respawn", btnSize))
    {
        Player* player = world->player;
        player->health = player->maxHealth;
        Unpause(state);
        TeleportHome(state, world);
    }

    ImGui::SetCursorPos(ImVec2(cursorPos.x + (btnSize.x / 2) + 2, cursorPos.y));

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

    int numPerLine[] = { 3, 3, 2 };
    int curBiome = 0;

    ImVec2 baseCursor = ImGui::GetCursorPos();
    float biomeGap = 85.0f;

    for (int n = 0; n < ArrayCount(numPerLine); n++)
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
            BeginLoadingScreen(state, 0.5f, RegenerateWorldCallback);
        }
    }

    ImGui::SetCursorPos(ImVec2(loc.x + (btnSize.x * 0.5f) + 3.0f, loc.y));

    if (ImGui::Button("Go Back", btnSize))
        state->pauseState = PAUSED;

    ImGui::End();
}

static void CreateCommandUI(GameState* state)
{
    CommandProcessor& processor = state->cmdProcessor;

    if (processor.errorTime > 0.0f)
    {   
        processor.errorTime -= state->deltaTime;
        ImVec2 textSize = ImGui::CalcTextSize(processor.error);
        CenteredUIWindow(textSize.x + 15.0f, textSize.y, 0.0f, 115.0f);
        ImGui::Begin("CommandError", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(processor.error);
        ImGui::PopStyleColor();
        ImGui::End();
    }

    float width = 300.0f;
    ImVec2 size = CenteredUIWindow(width, 100.0f, 0.0f, 200.0f);

    ImGui::Begin("EnterCommand", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav);
    WindowHeader("Enter a Command", size.x);

    MultiSpacing(2);

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    ImGui::PushItemWidth(width);
    ImGui::InputText("##hidelabel", processor.inputText, sizeof(processor.inputText));
    ImGui::PopItemWidth();

    ImVec2 btnSize = ImVec2(85.0f, 25.0f);
    ImVec2 loc = GetButtonLoc(btnSize, size.x, size.y - 35.0f);
    ImGui::SetCursorPos(ImVec2(loc.x - (btnSize.x * 0.5f) - 3.0f, loc.y));

    if (ImGui::Button("Submit", btnSize) || KeyPressed(state->input, KEY_ENTER))
    {
        CommandResult result = ProcessCommand(state, processor);

        if (result.error == nullptr)
        {
            if (result.newState != PLAYING)
                state->pauseState = (PauseState)result.newState;
            else Unpause(state);
        }
        else
        {
            processor.error = result.error;
            processor.errorTime = 3.0f;
        }
    }

    ImGui::SetCursorPos(ImVec2(loc.x + (btnSize.x * 0.5f) + 3.0f, loc.y));

    if (ImGui::Button("Cancel", btnSize))
        Unpause(state);

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

    ImVec2 windowSize = ImVec2((float)size.x, (float)size.y);
    ImGui::SetNextWindowSize(windowSize);

    ImGui::Begin("Profiler", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);

    double fps = GetFPS();

    char fpsText[16];
    sprintf(fpsText, "FPS: %.1f", fps);

    MultiSpacing(4);
    ImGui::Text(fpsText);

    #if DEBUG_SERVICES

    ImGui::Spacing();

    DebugTable& t = g_debugTable;

    float laneHeight = 5.0f;
    int laneCount = t.chartLaneCount;
    float barHeight = laneHeight * laneCount;
    float spacing = barHeight + 30.0f;
    float chartWidth = 800.0f;
    float chartMinX = 100.0f;
    float scale = t.chartScale * chartWidth;

    ImVec2 mouse = ImGui::GetMousePos();

    ImColor colors[] =
    {
        ImColor(255, 0, 0, 255),
        ImColor(0, 255, 0, 255),
        ImColor(128, 180, 255, 255),
        ImColor(255, 255, 0, 255),
        ImColor(0, 255, 255, 255),
        ImColor(255, 0, 255, 255)
    };

    int colorCount = ArrayCount(colors);

    ImU32 white = ImGui::ColorConvertFloat4ToU32(ImColor(255, 255, 255, 255));

    char hoverText[128];
    bool hasHoverText = false;
    ImVec2 hoverStart, hoverEnd;

    ImVec2 leftBarStart = ImVec2(chartMinX - 0.5f, 0);
    ImVec2 leftBarEnd = ImVec2(chartMinX + 0.5f, windowSize.y - 1.0f);
    ImGui::GetWindowDrawList()->AddRectFilled(leftBarStart, leftBarEnd, white);

    for (int i = 0; i < t.frames.size(); i++)
    {
        DebugFrame& frame = t.frames[i];

        float stackY = spacing * i;

        for (int j = 0; j < frame.regions.size(); j++)
        {
            FrameRegion& region = frame.regions[j];
            ImU32 color = ImGui::ColorConvertFloat4ToU32(colors[j % colorCount]);

            float minX = chartMinX + scale * region.minT;
            float maxX = chartMinX + scale * region.maxT;

            ImVec2 start = ImVec2(minX, stackY + region.laneIndex * laneHeight);
            ImVec2 end = ImVec2(maxX, start.y - laneHeight);
            ImGui::GetWindowDrawList()->AddRectFilled(start, end, color);

            if (ImGui::IsMouseHoveringRect(ImVec2(start.x, end.y), ImVec2(end.x, start.y)))
            {
                DebugRecord& record = region.record;
                sprintf(hoverText, "%s (%d): %.0f\n", record.func, record.line, region.elapsed);
                hoverStart = start;
                hoverEnd = end;
                hasHoverText = true;

                if (ImGui::IsMouseClicked(0))
                {
                    t.scopeToRecord = &record;
                    t.profilerState = PROFILER_RECORD_ONCE;
                }
            }
        }
    }

    ImVec2 rightBarStart = ImVec2((chartMinX + chartWidth) - 0.5f, 0);
    ImVec2 rightBarEnd = ImVec2((chartMinX + chartWidth) + 0.5f, windowSize.y - 1.0f);
    ImGui::GetWindowDrawList()->AddRectFilled(rightBarStart, rightBarEnd, white);

    if (hasHoverText)
    {
        ImVec2 textSize = ImGui::CalcTextSize(hoverText);
        float xPos = Min(mouse.x + 15.0f, windowSize.x - textSize.x - 15.0f);

        ImGui::SetCursorPos(ImVec2(xPos, hoverStart.y + 5.0f));
        ImGui::Text(hoverText);
    }

    if (ImGui::IsMouseClicked(1))
    {
        t.scopeToRecord = nullptr;

        if (t.profilerState == PROFILER_STOPPED)
            t.profilerState = PROFILER_RECORD_ONCE;
    }

    #endif

    ImGui::End();
}

static void CreateHUD(GameState* state, World* world)
{
    TIMED_FUNCTION;

    ivec2 size = FramebufferSize();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2((float)size.x, (float)size.y));

    ImGui::Begin("HUD", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);

    Player* player = world->player;

    char healthText[12];
    sprintf(healthText, "Health: %d", player->health);

    ImGui::Text(healthText);

    if (state->debugDisplay == DEBUG_DISPLAY_HUD)
    {
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
            char* name = GetName(world, block);

            ivec3 lwP = world->cursorBlockPos;
            WorldP p = LWorldToWorldP(world, lwP);

            char blockText[128];
            sprintf_s(blockText, 64, "Block: %s, Position: %i, %i, %i (%i, %i, %i)", name, lwP.x, lwP.y, lwP.z, p.x, p.y, p.z);

            ImGui::Text(blockText);
        }
    }

    ImGui::End();
}

static void RenderUI(GameState* state, Renderer& rend, UI& ui)
{
    TIMED_FUNCTION;

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

        case ENTERING_COMMAND:
            CreateCommandUI(state);
            break;

        case PLAYER_DEAD:
            CreateDeathUI(state, world, window);
            break;
    }

    CreateHUD(state, world);

    switch (state->debugDisplay)
    {
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
