// Voxel Engine
// Jason Bricco

// BUGS/TODO:

// Look Into:
// Frustum culling.
// GLSL Subroutines for optimizing shaders.
// Shader precompiling.

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLEW_STATIC
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <time.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "FastNoiseSIMD.h"

#define PROFILING 0
#define PROFILING_ONCE 1
#define ASSERTIONS 1
#define DEBUG_MEMORY 0

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_ONLY_PNG

#include "stb_image.h"
#include "stretchy_buffer.h"

#include <unordered_map>
#include <queue>
#include <fstream>

using namespace std;

#if ASSERTIONS

static void HandleAssertion(char* file, int line)
{
	char buffer[128];
	sprintf(buffer, "An assertion was triggered in file %s, line %i.\nAbort for debugging?", file, line);
	int val = MessageBox(NULL, buffer, NULL, MB_YESNOCANCEL | MB_ICONERROR);
	
	if (val == IDNO) exit(-1);
	else if (val == IDYES) abort();
}

#define Assert(expression) if (!(expression)) { HandleAssertion(__FILE__, __LINE__); }
#else
#define Assert(expression)
#endif

#define Malloc(type) (type*)malloc(sizeof(type))
#define Calloc(type) (type*)calloc(1, sizeof(type))

#if DEBUG_MEMORY
#define _CRTDBG_MAP_ALLOC 1
#include <crtdbg.h>  
#endif

static bool g_paused;

#include "profiling.h"
#include "intrinsics.h"
#include "utils.h"
#include "vec.h"
#include "matrix.h"
#include "async.h"
#include "logging.h"
#include "input.h"
#include "mesh.h"
#include "world.h"
#include "renderer.h"
#include "shaders.h"
#include "simulation.h"

#include "matrix.cpp"
#include "async.cpp"
#include "input.cpp"
#include "shaders.cpp"
#include "renderer.cpp"
#include "mesh.cpp"
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
			SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, 
				mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, 
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(window, &g_windowPos);
		SetWindowPos(window, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | 
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
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

static void CheckWorld(World* world, Player* player)
{
	Vec3 pos = player->pos;
	float min = world->pMin, max = world->pMax;

	if (pos.x < min) 
	{
		player->pos.x = max - (min - pos.x);
		world->refX--;
		ShiftWorld(world);
	}
	else if (pos.x > max) 
	{
		player->pos.x = min + (pos.x - max);
		world->refX++;
		ShiftWorld(world);
	}

	if (pos.z < min) 
	{
		player->pos.z = max - (min - pos.z);
		world->refZ--;
		ShiftWorld(world);
	}
	else if (pos.z > max) 
	{
		player->pos.z = min + (pos.z - max);
		world->refZ++;
		ShiftWorld(world);
	}
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

	if (g_paused) return;

	if (KeyPressed(KEY_T))
		ToggleFullscreen(glfwGetWin32Window(window));

	CheckWorld(world, player);

	double mouseX, mouseY;

	glfwGetCursorPos(window, &mouseX, &mouseY);

	Renderer* rend = (Renderer*)glfwGetWindowUserPointer(window);

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
	#if DEBUG_MEMORY

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);

	#endif

	// CreateThreads();

	srand((uint32_t)time(0));

	Renderer* rend = new Renderer();

	GLFWwindow* window = InitRenderer(rend);

	glfwSetKeyCallback(window, OnKey);
	glfwSetWindowSizeCallback(window, SetWindowSize);
	glfwSetMouseButtonCallback(window, OnMouseButton);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, rend->windowWidth / 2.0f, rend->windowHeight / 2.0f);

	World* world = NewWorld(8);

	Player* player = NewPlayer(world->pMin, world->pMax);
	rend->camera = player->camera;
	
	double lastTime = glfwGetTime();

	float deltaTime = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		BEGIN_TIMED_BLOCK(GAME_LOOP);

		ResetInput();
		glfwPollEvents();

		Update(window, player, world, deltaTime);
		RenderScene(rend, world);

		glfwSwapBuffers(window);

		#if PROFILING

		ShowFPS(window);

		#endif

		double endTime = glfwGetTime();
		deltaTime = (float)(endTime - lastTime);
		lastTime = endTime;

		END_TIMED_BLOCK(GAME_LOOP);
		FLUSH_COUNTERS();
	}

	glfwTerminate();
	return 0;
}
