//
// Gamecraft
//

#if _DEBUG
static char* g_buildType = "DEBUG";
#else
static char* g_buildType = "RELEASE";
#endif

#define SWAP_INTERVAL 1

static int g_buildID = 304;

#pragma warning(push, 0)

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLEW_STATIC
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shlwapi.h>
#include <xaudio2.h>
#include <time.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "FastNoiseSIMD.h"
#include "imgui.h"

#include "stb_vorbis.h"

#include <vector>
#include <queue>
#include <fstream>
#include <functional>
#include <algorithm>
#include <atomic>
#include <stack>

#define GLM_FORCE_AVX2
#define GLM_FORCE_INLINE
#define GLM_FORCE_NO_CTOR_INIT
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/fwd.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/norm.hpp"

using namespace glm;
using namespace std;

#pragma warning(pop)

#define Unused(x) ((void)(x))

#if _DEBUG
#define Print(...) { \
    char print_buffer[256]; \
    snprintf(print_buffer, sizeof(print_buffer), __VA_ARGS__); \
    OutputDebugString(print_buffer); \
}
#else
#define Print(...) { \
    char print_buffer[256]; \
    snprintf(print_buffer, sizeof(print_buffer), __VA_ARGS__); \
    MessageBox(NULL, print_buffer, NULL, MB_OK | MB_ICONINFORMATION); \
}
#endif

#if _DEBUG
#define Error(...) { \
    char error_buffer[256]; \
    snprintf(error_buffer, sizeof(error_buffer), __VA_ARGS__); \
    OutputDebugString(error_buffer); \
    DebugBreak(); \
}
#else
#define Error(...) { \
    char error_buffer[256]; \
    snprintf(error_buffer, sizeof(error_buffer), __VA_ARGS__); \
    MessageBox(NULL, error_buffer, NULL, MB_OK | MB_ICONERROR); \
    exit(-1); \
}
#endif

#include "Config.h"
#include "Utils.h"
#include "ObjectPool.h"
#include "Containers.h"
#include "Random.h"
#include "Debug.h"
#include "UI.h"
#include "Audio.h"
#include "FileHelper.h"
#include "AssetBuilder.h"
#include "Assets.h"
#include "Input.h"
#include "Mesh.h"
#include "Block.h"
#include "Particles.h"
#include "Environment.h"
#include "Generation.h"
#include "World.h"
#include "WorldIO.h"
#include "Renderer.h"
#include "Lighting.h"
#include "WorldRender.h"
#include "Simulation.h"
#include "Async.h"
#include "Commands.h"
#include "Gamestate.h"

static void Pause(GameState* state, PauseState pauseState);
static void Unpause(GameState* state);
static void BeginLoadingScreen(GameState* state, float fadeTime, function<void(GameState*, World*)> callback);
static inline ivec2 FramebufferSize();
static inline void CenterCursor();

#include "Debug.cpp"
#include "Audio.cpp"
#include "Assets.cpp"
#include "Async.cpp"
#include "Input.cpp"
#include "Mesh.cpp"
#include "Renderer.cpp"
#include "Block.cpp"
#include "World.cpp"
#include "Lighting.cpp"
#include "WorldRender.cpp"
#include "Particles.cpp"
#include "Dungeon.cpp"
#include "Environment.cpp"
#include "Generation.cpp"
#include "WorldIO.cpp"
#include "Simulation.cpp"
#include "UI.cpp"
#include "Commands.cpp"

static GLFWwindow* window;

// Window placement for fullscreen toggling.
static WINDOWPLACEMENT windowPos = { sizeof(windowPos) };

static inline ivec2 FramebufferSize()
{
	int displayW, displayH;
    glfwGetFramebufferSize(window, &displayW, &displayH);
    return ivec2(displayW, displayH);
}

static void ToggleFullscreen(HWND wnd)
{
	DWORD style = GetWindowLong(wnd, GWL_STYLE);

	if (style & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO mi = { sizeof(mi) };

		if (GetWindowPlacement(wnd, &windowPos) && GetMonitorInfo(MonitorFromWindow(wnd, MONITOR_DEFAULTTOPRIMARY), &mi))
		{
			SetWindowLong(wnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(wnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, 
				mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(wnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(wnd, &windowPos);
		SetWindowPos(wnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

static void Pause(GameState* state, PauseState pauseState)
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	ChangeVolume(&state->audio, 0.25f, 0.5f);
	Renderer& rend = state->renderer;
	rend.fadeColor.a = rend.fadeColor.a > 0.5f ? 0.9f : 0.75f;
	state->pauseState = pauseState;
}

static inline void CenterCursor()
{
	ivec2 size = FramebufferSize();
	glfwSetCursorPos(window, size.x * 0.5f, size.y * 0.5f);
}

static void Unpause(GameState* state)
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	ChangeVolume(&state->audio, 0.75f, 0.5f);
	Renderer& rend = state->renderer;
	rend.fadeColor.a = 0.0f;
	CenterCursor();
	state->pauseState = PLAYING;
}

static void BeginLoadingScreen(GameState* state, float fadeTime, function<void(GameState*, World*)> callback)
{
	if (state->pauseState != PLAYING)
		Unpause(state);

	LoadingInfo& info = state->loadInfo;
	info.state = LOADING_FADE_IN;
	info.initialFade = state->renderer.fadeColor;
	info.t = 0.0f;
	info.fadeTime = fadeTime;
	info.callback = callback;
	state->pauseState = LOADING;
}

static void ProcessLoadingScreen(GameState* state, World* world, float deltaTime)
{
	LoadingInfo& info = state->loadInfo;
	Renderer& rend = state->renderer;

	if (info.state == LOADING_FADE_IN)
	{
		rend.fadeColor = mix(info.initialFade, BLACK_COLOR, info.t);
		info.t += deltaTime / info.fadeTime;

		if (info.t >= 1.0f)
		{
			rend.fadeColor = BLACK_COLOR;
			info.state = LOADING_NONE;
		}
	}
	else if (info.state == LOADING_FADE_OUT)
	{
		rend.fadeColor = mix(BLACK_COLOR, CLEAR_COLOR, info.t);
		info.t += deltaTime / info.fadeTime;

		if (info.t >= 1.0f)
		{
			rend.fadeColor = CLEAR_COLOR;
			CenterCursor();
			state->pauseState = PLAYING;
		}
	}
	else
	{
		if (HasBackgroundWork(world))
			return;

		info.callback(state, world);
		info.t = 0.0f;
		info.state = LOADING_FADE_OUT;
	}
}

static void OnGamePlaying(GameState* state)
{
	if (KeyPressed(state->input, KEY_ESCAPE))
		Pause(state, PAUSED);

	if (KeyPressed(state->input, KEY_SLASH) && state->pauseState != ENTERING_COMMAND)
		Pause(state, ENTERING_COMMAND);

	if (KeyPressed(state->input, KEY_E))
		Pause(state, SELECTING_BLOCK);

	if (KeyPressed(state->input, KEY_T))
		ToggleFullscreen(glfwGetWin32Window(window));
}

static void OnGamePaused(GameState* state)
{
	if (KeyPressed(state->input, KEY_ESCAPE))
		Unpause(state);
}

static void OnSelectingBlock(GameState* state)
{
	if (KeyPressed(state->input, KEY_ESCAPE) || KeyPressed(state->input, KEY_E))
		Unpause(state);
}

static void Update(GameState* state, Player* player, World* world, float deltaTime)
{
	Input& input = state->input;

	switch (state->pauseState)
	{
		case LOADING:
			ProcessLoadingScreen(state, world, deltaTime);
			break;

		case PLAYING:
			OnGamePlaying(state);
			break;

		case PAUSED:
		case ENTERING_COMMAND:
			OnGamePaused(state);
			break;

		case SELECTING_BLOCK:
			OnSelectingBlock(state);
			break;
	}

	if (KeyPressed(input, KEY_F1))
		state->debugDisplay = !state->debugDisplay;

	UpdateAudio(&state->audio, deltaTime);
	UpdateWorld(state, world, state->camera, player);
	UpdateEnvironment(state, world, deltaTime);

	if (state->pauseState != PLAYING || !player->spawned) 
		return;

	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		ivec2 size = FramebufferSize();
		double cX = size.x * 0.5f, cY = size.y * 0.5f;

		glfwSetCursorPos(window, cX, cY);

		Camera* cam = state->camera;

		float rotX = (float)(cX - mouseX) * cam->sensitivity;
		float rotY = (float)(cY - mouseY) * cam->sensitivity;
		RotateCamera(cam, rotX, rotY);
	}

	Simulate(state, world, player, deltaTime);
}

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	if (!glfwInit())
		Error("GLFW failed to initialize.\n");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	int screenWidth = 1024, screenHeight = 768;

	window = glfwCreateWindow(screenWidth, screenHeight, "Gamecraft", NULL, NULL);

	if (window == nullptr)
		Error("Failed to create window.\n");

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		Error("Failed to initialize GLEW.\n");

	glewExperimental = GL_TRUE;

	glfwSwapInterval(SWAP_INTERVAL);

	srand((uint32_t)time(0));

	GameState* state = new GameState();

	DEBUG_INIT();

	char savePath[MAX_PATH];
	state->savePath = PathToExe("Saves", savePath, MAX_PATH);
	CreateDirectory(state->savePath, NULL);

	InitUI(window, state->ui);
	InitAudio(&state->audio);
	
	CreateThreads(state);
	LoadAssets(state);

	Camera* cam = NewCamera();
	state->camera = cam;

	Renderer& rend = state->renderer;
	InitRenderer(state, rend, screenWidth, screenHeight);

	glfwSetWindowUserPointer(window, state);
	glfwSetKeyCallback(window, OnKey);
	glfwSetFramebufferSizeCallback(window, SetWindowSize);
	glfwSetMouseButtonCallback(window, OnMouseButton);
	glfwSetCharCallback(window, InputCharCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, screenWidth / 2.0f, screenHeight / 2.0f);

	WorldConfig worldConfig = {};
	worldConfig.radius = 1024;

	World* world = NewWorld(state, 8, worldConfig);

	Player* player = NewPlayer(state);
	world->player = player;

	int fW, fH;
	glfwGetFramebufferSize(window, &fW, &fH);
	SetWindowSize(window, fW, fH);

	LoadGameSettings(state);
	
	double lastTime = glfwGetTime();
	float deltaTime = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		FRAME_MARKER;
		BEGIN_BLOCK(HANDLE_INPUT);

		ResetInput(state->input);
		glfwPollEvents();

		END_BLOCK(HANDLE_INPUT);

		if (!state->minimized)
		{
			BEGIN_BLOCK(BEGIN_UI);

			BeginNewUIFrame(window, state->ui, deltaTime);
			CreateUI(state, window, world, worldConfig);

			END_BLOCK(BEGIN_UI);
			BEGIN_BLOCK(GAME_UPDATE);

			RunAsyncCallbacks(state);
			Update(state, player, world, deltaTime);

			END_BLOCK(GAME_UPDATE);
			BEGIN_BLOCK(GAME_RENDER);

			RenderScene(state, rend, cam);

			END_BLOCK(GAME_RENDER);
		}

		glfwSwapBuffers(window);
		DEBUG_END_FRAME(state);

		double endTime = glfwGetTime();
		deltaTime = Min((float)(endTime - lastTime), 0.0666f);
		state->deltaTime = deltaTime;
		state->time += deltaTime;
		lastTime = endTime;
	}

	SaveWorld(state, world);
	SaveGameSettings(state);

	glfwTerminate();

	return 0;
}
