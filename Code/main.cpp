//
// Jason Bricco
//

#pragma warning(push, 0)

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLEW_STATIC
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shlwapi.h>
#include <xaudio2.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <time.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "FastNoiseSIMD.h"
#include "imgui.h"

#include "stb_vorbis.h"

#include <unordered_map>
#include <fstream>
#include <atomic>
#include <string>
#include <mutex>
#include <algorithm>
#include <queue>

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

#define ENABLE_PRINT 1

#if ENABLE_PRINT
#define Print(...) { \
    char print_buffer[256]; \
    snprintf(print_buffer, sizeof(print_buffer), __VA_ARGS__); \
    OutputDebugString(print_buffer); \
}
#else
#define Print(...)
#endif

static bool g_paused;

#include "memory.h"
#include "random.h"
#include "utils.h"
#include "profiling.h"
#include "ui.h"
#include "audio.h"
#include "filehelper.h"
#include "assetbuilder.h"
#include "assets.h"
#include "input.h"
#include "mesh.h"
#include "block.h"
#include "world.h"
#include "worldio.h"
#include "renderer.h"
#include "worldrender.h"
#include "simulation.h"
#include "async.h"
#include "generation.h"
#include "particles.h"
#include "gamestate.h"

static void Pause(GLFWwindow* window, World* world);
static void Unpause(GLFWwindow* window);

#include "audio.cpp"
#include "assets.cpp"
#include "async.cpp"
#include "input.cpp"
#include "mesh.cpp"
#include "renderer.cpp"
#include "block.cpp"
#include "world.cpp"
#include "worldrender.cpp"
#include "generation.cpp"
#include "worldio.cpp"
#include "simulation.cpp"
#include "particles.cpp"
#include "ui.cpp"

#if _DEBUG
static char* buildType = "DEBUG";
#else
static char* buildType = "RELEASE";
#endif

static char* buildID = "150";

// Window placement for fullscreen toggling.
static WINDOWPLACEMENT windowPos = { sizeof(windowPos) };

static GameState state;

static void ToggleFullscreen(HWND window)
{
	DWORD style = GetWindowLong(window, GWL_STYLE);

	if (style & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO mi = { sizeof(mi) };

		if (GetWindowPlacement(window, &windowPos) && GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &mi))
		{
			SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, 
				mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(window, &windowPos);
		SetWindowPos(window, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

static void ShowFPS(GLFWwindow* window)
{
	static double prevSec = 0.0;
	static int frameCount = 0;

	double current = glfwGetTime();
	double elapsed = current - prevSec;

	if (elapsed > 0.25)
	{
		prevSec = current;
		double fps = (double)frameCount / elapsed;

		char profile[64];
		sprintf(profile, "Gamecraft - FPS: %.01f - %s - Build: %s\n", fps, buildType, buildID);

		glfwSetWindowTitle(window, profile);
		frameCount = 0;
	}

	frameCount++;
}

static void Pause(GLFWwindow* window, World* world)
{
	SaveWorld(world);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	ChangeVolume(&state.audio, 0.25f, 0.5f);
	Camera* cam = state.camera;
	cam->fadeColor.a = cam->fadeColor.a > 0.5f ? 0.9f : 0.75f;
	g_paused = true;
}

static void Unpause(GLFWwindow* window)
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	ChangeVolume(&state.audio, 0.75f, 0.5f);
	Camera* cam = state.camera;
	cam->fadeColor.a = 0.0f;
	glfwSetCursorPos(window, state.windowWidth * 0.5f, state.windowHeight * 0.5f);
	g_paused = false;
}

static void Update(GLFWwindow* window, Player* player, World* world, float deltaTime)
{
	Input& input = state.input;

	if (KeyPressed(input, KEY_ESCAPE))
	{
		if (g_paused)
			Unpause(window);
		else Pause(window, world);
	}

	if (KeyPressed(input, KEY_T))
		ToggleFullscreen(glfwGetWin32Window(window));

	UpdateAudio(&state.audio, deltaTime);
	UpdateWorld(&state, world, state.camera, player);

	if (g_paused || !player->spawned) return;

	if (KeyPressed(input, KEY_EQUAL))
	{
		state.ambient = state.ambient == 0.0f ? 1.0f : 0.0f;
		vec3 newClear = state.clearColor * state.ambient;
		glClearColor(newClear.r, newClear.g, newClear.b, 1.0f);
	}

	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	double cX = state.windowWidth * 0.5f, cY = state.windowHeight * 0.5f;
	Camera* cam = state.camera;

	float rotX = (float)(cX - mouseX) * cam->sensitivity;
	float rotY = (float)(cY - mouseY) * cam->sensitivity;

	RotateCamera(cam, rotX, rotY);
	glfwSetCursorPos(window, cX, cY);

	Simulate(&state, world, player, deltaTime);

	state.rain.pos = vec3(player->pos.x, player->pos.y + 50.0f, player->pos.z);
	UpdateParticles(state.rain, world, deltaTime);
}

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	if (!glfwInit())
		Error("GLFW failed to initialize.\n");

	// Window creation.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	int screenWidth = 1024, screenHeight = 768;

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Gamecraft", NULL, NULL);

	if (window == nullptr)
		Error("Failed to create window.\n");

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		Error("Failed to initialize GLEW.\n");

	glewExperimental = GL_TRUE;

	// Set vertical synchronization to the monitor refresh rate.
	glfwSwapInterval(1);

	InitUI(window, state.ui);
	InitAudio(&state.audio);
	
	CreateThreads(&state);
	LoadAssets(&state);

	Camera* cam = NewCamera();
	state.camera = cam;

	InitRenderer(&state, cam, screenWidth, screenHeight);

	glfwSetWindowUserPointer(window, &state);
	SetWindowSize(window, screenWidth, screenHeight);

	glfwSetKeyCallback(window, OnKey);
	glfwSetWindowSizeCallback(window, SetWindowSize);
	glfwSetMouseButtonCallback(window, OnMouseButton);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, state.windowWidth / 2.0f, state.windowHeight / 2.0f);

	InitParticleEmitter(state.rain, 6, 20.0f);

	World* world = NewWorld(&state, 13, 1024);

	Player* player = NewPlayer();
	world->player = player;
	
	double lastTime = glfwGetTime();

	float deltaTime = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		BEGIN_TIMED_BLOCK(GAME_LOOP);

		ResetInput(state.input);
		glfwPollEvents();

		BeginNewUIFrame(window, state.ui, deltaTime);
		CreateUI(window, &state);
		Update(window, player, world, deltaTime);
		RenderScene(&state, cam);

		END_TIMED_BLOCK(GAME_LOOP);
		FLUSH_COUNTERS();

		glfwSwapBuffers(window);

		ShowFPS(window);

		double endTime = glfwGetTime();
		deltaTime = Min((float)(endTime - lastTime), 0.0666f);
		cam->animTime = fmodf(cam->animTime + (deltaTime * 0.5f), 1.0f);
		lastTime = endTime;
	}

	SaveWorld(world);
	glfwTerminate();

	FLUSH_COUNTERS();

	return 0;
}
