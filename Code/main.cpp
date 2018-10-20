//
// Jason Bricco
//

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLEW_STATIC
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <Shlwapi.h>
#include <time.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "FastNoiseSIMD.h"

#define DEBUG_MEMORY 0
#define PROFILING 0
#define PROFILING_ONCE 0

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG

#include "stb_image.h"
#include "stretchy_buffer.h"

#include <unordered_map>
#include <fstream>
#include <atomic>

#define GLM_FORCE_INLINE
#define GLM_FORCE_NO_CTOR_INIT

#include "glm/fwd.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

using namespace glm;
using namespace std;

#if PROFILING || DEBUG_MEMORY

#include <string>
#include <mutex>

#endif

#define Print(...) { \
    char buffer[256]; \
    snprintf(buffer, sizeof(buffer), __VA_ARGS__); \
    OutputDebugString(buffer); \
}

static bool g_paused;

#include "profiling.h"
#include "intrinsics.h"
#include "filehelper.h"
#include "assets.h"
#include "random.h"
#include "utils.h"
#include "input.h"
#include "uniforms.h"
#include "mesh.h"
#include "world.h"
#include "renderer.h"
#include "simulation.h"
#include "async.h"

#include "assets.cpp"
#include "async.cpp"
#include "input.cpp"
#include "mesh.cpp"
#include "renderer.cpp"
#include "world.cpp"
#include "simulation.cpp"

// Window placement for fullscreen toggling.
static WINDOWPLACEMENT g_windowPos = { sizeof(g_windowPos) };

static void ToggleFullscreen(HWND window)
{
	DWORD style = GetWindowLong(window, GWL_STYLE);

	if (style & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO mi = { sizeof(mi) };

		if (GetWindowPlacement(window, &g_windowPos) && 
			GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &mi))
		{
			SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(window, &g_windowPos);
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
		sprintf(profile, "Voxel Engine     FPS: %.01f\n", fps);

		glfwSetWindowTitle(window, profile);
		frameCount = 0;
	}

	frameCount++;
}

static void Update(GLFWwindow* window, Player* player, World* world, float deltaTime)
{
	if (KeyPressed(KEY_ESCAPE))
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		g_paused = true;
	}

	if (g_paused && MousePressed(0))
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		g_paused = false;
		return;
	}

	Renderer* rend = (Renderer*)glfwGetWindowUserPointer(window);
	UpdateWorld(world, rend, player);

	if (g_paused) return;

	if (KeyPressed(KEY_T))
		ToggleFullscreen(glfwGetWin32Window(window));

	double mouseX, mouseY;

	glfwGetCursorPos(window, &mouseX, &mouseY);

	double cX = rend->windowWidth / 2.0, cY = rend->windowHeight / 2.0f;

	Camera* cam = player->camera;

	float rotX = (float)(cX - mouseX) * cam->sensitivity;
	float rotY = (float)(cY - mouseY) * cam->sensitivity;

	RotateCamera(cam, rotX, rotY);
	glfwSetCursorPos(window, cX, cY);

	Simulate(rend, world, player, deltaTime);
}

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
	if (!glfwInit())
	{
		OutputDebugString("GLFW failed to initialize.");
		return -1;
	}

	// Window creation.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	int screenWidth = 1024, screenHeight = 768;

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Gamecraft", NULL, NULL);

	if (window == nullptr)
	{
		OutputDebugString("Failed to create window.");
		return -2;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		OutputDebugString("Failed to initialize GLEW.");
		return -3;
	}

	glewExperimental = GL_TRUE;

	// Set vertical synchronization to the monitor refresh rate.
	glfwSwapInterval(1);

	glClearColor(0.53f, 0.80f, 0.92f, 1.0f);
	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	#if ASSERTIONS

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback((GLDEBUGPROC)OnOpenGLMessage, 0);

	#endif
	
	CreateThreads();

	Assets assets;
	LoadAssets(&assets);

	Renderer* rend = Calloc<Renderer>();
	InitRenderer(rend, &assets, screenWidth, screenHeight);

	glfwSetWindowUserPointer(window, rend);
	SetWindowSize(window, screenWidth, screenHeight);

	glfwSetKeyCallback(window, OnKey);
	glfwSetWindowSizeCallback(window, SetWindowSize);
	glfwSetMouseButtonCallback(window, OnMouseButton);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, rend->windowWidth / 2.0f, rend->windowHeight / 2.0f);

	World* world = NewWorld(6, 4);

	Player* player = NewPlayer(world->pBounds, rend->camera);
	rend->camera = player->camera;
	
	double lastTime = glfwGetTime();

	float deltaTime = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		BEGIN_TIMED_BLOCK(GAME_LOOP);

		ResetInput();
		glfwPollEvents();

		Update(window, player, world, deltaTime);
		RenderScene(rend, &assets);

		END_TIMED_BLOCK(GAME_LOOP);
		FLUSH_COUNTERS();

		glfwSwapBuffers(window);

		ShowFPS(window);

		double endTime = glfwGetTime();
		deltaTime = Min((float)(endTime - lastTime), 0.0666f);
		rend->animTime = fmodf(rend->animTime + deltaTime, 100.0f);
		lastTime = endTime;

		#if DEBUG_MEMORY

		if (KeyPressed(KEY_BACKSLASH))
			LogMemoryStatus();

		#endif
	}

	SaveWorld(world);
	glfwTerminate();
	FLUSH_COUNTERS();

	return 0;
}
