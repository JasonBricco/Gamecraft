// Voxel Engine
// Jason Bricco

// BUGS/TODO:
// Some bug making blocks appear in another chunk when you place them sometimes.

// Look Into:
// Frustum culling.
// When we frustum cull, we can ensure we only load chunks within the frustum and a short
// distance around the player. If we already created it and look away, just stop rendering it but leave
// it as is. We will have freed the other mesh list data and so the memory footprint won't be too huge.
// GLSL Subroutines for optimizing shaders.
// Shader precompiling.
// Noise "quadrants" (actually larger) to solve noise precision issue. 

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
#define PROFILING_ONCE 0
#define ASSERTIONS 1
#define DEBUG_MEMORY 0

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_ONLY_PNG

#include "stb_image.h"
#include "stretchy_buffer.h"

#include <unordered_map>
#include <fstream>
#include <atomic>

#define GLM_FORCE_AVX2
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

#if DEBUG_MEMORY

#include <string>
#include <mutex>

#define Malloc(type, size, id) (type*)DebugMalloc(id, size)
#define Calloc(type, size, id) (type*)DebugCalloc(id, size)
#define Free(ptr, id) DebugFree(id, ptr)

#else

#define Malloc(type, size, id) (type*)malloc(size)
#define Calloc(type, size, id) (type*)calloc(1, size)
#define Free(ptr, id) free(ptr)

#endif

static bool g_paused;

#include "profiling.h"
#include "intrinsics.h"
#include "random.h"
#include "utils.h"
#include "fileio.h"
#include "input.h"
#include "mesh.h"
#include "block.h"
#include "world.h"
#include "renderer.h"
#include "shaders.h"
#include "simulation.h"
#include "async.h"

#include "async.cpp"
#include "input.cpp"
#include "shaders.cpp"
#include "mesh.cpp"
#include "renderer.cpp"
#include "block.cpp"
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
		SetWindowPos(window, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER 
			| SWP_FRAMECHANGED);
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

	UpdateWorld(world, player);

	if (g_paused) return;

	if (KeyPressed(KEY_T))
		ToggleFullscreen(glfwGetWin32Window(window));

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
	CreateThreads();

	Renderer* rend = new Renderer();
	GLFWwindow* window = InitRenderer(rend);

	glfwSetKeyCallback(window, OnKey);
	glfwSetWindowSizeCallback(window, SetWindowSize);
	glfwSetMouseButtonCallback(window, OnMouseButton);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, rend->windowWidth / 2.0f, rend->windowHeight / 2.0f);

	World* world = NewWorld(6, 4);

	Player* player = NewPlayer(world->pBounds);
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

		END_TIMED_BLOCK(GAME_LOOP);
		FLUSH_COUNTERS();

		glfwSwapBuffers(window);

		ShowFPS(window);

		double endTime = glfwGetTime();
		deltaTime = Min((float)(endTime - lastTime), 0.0666f);
		lastTime = endTime;

		#if DEBUG_MEMORY

		if (KeyPressed(KEY_BACKSLASH))
		{
			string str;
			str.append("ALLOC INFO:\n");

			for (pair<string, uint32_t> element : g_allocInfo)
			{
				if (element.second > 0)
				{
					str.append("ID: " + element.first + "\n");
					str.append("Count: " + to_string(element.second) + "\n");
				}
			}

			str.append("\n");
			OutputDebugString(str.c_str());
		}

		#endif
	}

	glfwTerminate();

	return 0;
}
