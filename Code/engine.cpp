// Voxel Engine
// Jason Bricco

// BUGS/TODO:
// Need a crosshair to show where editing.
// Editing/deleting not working.
// Try using WinMain and fixing batch files.

// Soon:
// Frustum culling.

#include "stdafx.h"

#define PROFILING 1
#define ASSERTIONS 1
#define DEBUG_MEMORY 0

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_ONLY_PNG

#include "stb_image.h"

#if ASSERTIONS
#define Assert(expression) if (!(expression)) { abort(); }
#else
#define Assert(expression)
#endif

#define Malloc(type) (type*)malloc(sizeof(type))
#define Calloc(type) (type*)calloc(1, sizeof(type))

#if DEBUG_MEMORY
#define _CRTDBG_MAP_ALLOC 1
#include <crtdbg.h>  
#endif

#include "input.h"
#include "utils.h"
#include "shaders.h"
#include "renderer.h"
#include "world.h"
#include "simulation.h"
#include "globals.h"

#include "input.cpp"
#include "shaders.cpp"
#include "renderer.cpp"
#include "world.cpp"
#include "simulation.cpp"
 
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
	}

	if (g_paused) return;

	ivec3 pos = ToChunkPos(player->pos);

	UnloadChunks(world, pos);
	LoadSurroundingChunks(world, pos);

	if (KeyPressed(KEY_TAB))
		player->flying = !player->flying;

	double mouseX, mouseY;

	glfwGetCursorPos(window, &mouseX, &mouseY);

	double cX = WindowWidth() / 2.0, cY = WindowHeight() / 2.0f;

	Camera* cam = player->camera;

	float rotX = (float)(cX - mouseX) * cam->sensitivity;
	float rotY = (float)(cY - mouseY) * cam->sensitivity;

	RotateCamera(cam, rotX, rotY);
	glfwSetCursorPos(window, cX, cY);

	Simulate(world, player, deltaTime);
}

int main()
{
	#if DEBUG_MEMORY

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);

	#endif

	srand((uint32_t)time(0));

	GLFWwindow* window = InitRenderer();

	glfwSetKeyCallback(window, OnKey);
	glfwSetWindowSizeCallback(window, OnWindowResize);
	glfwSetMouseButtonCallback(window, OnMouseButton);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, WindowWidth() / 2.0f, WindowHeight() / 2.0f);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* vMode = glfwGetVideoMode(monitor);

	World* world = NewWorld(512, 512);

	Player* player = NewPlayer(world->spawn);
	SetCamera(player->camera);
	
	int refreshHz = vMode->refreshRate * 2;
	float targetSeconds = 1.0f / refreshHz;
	double lastTime = glfwGetTime();

	float deltaTime = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		ResetInput();
		glfwPollEvents();

		Update(window, player, world, deltaTime);
		
		PreRender();
		
		mat4 model;

		for (int i = 0; i < world->loadedCount; i++)
		{
			Chunk* next = world->loadedChunks[i];

			if (next->state == CHUNK_BUILT)
			{
				model = translate(mat4(), (vec3)next->wPos);
				SetUniform(0, "model", model);
				DrawChunk(next);
			}
		}

		double secondsElapsed = glfwGetTime() - lastTime;

		while (secondsElapsed < targetSeconds)
		{
			DWORD msLeft = (DWORD)(1000.0f * (targetSeconds - secondsElapsed));
			if (msLeft > 0) Sleep(msLeft);

			while (secondsElapsed < targetSeconds)                         
            	secondsElapsed = glfwGetTime() - lastTime;
		}

		glfwSwapBuffers(window);

		#if PROFILING

		ShowFPS(window);

		#endif

		double endTime = glfwGetTime();
		deltaTime = (float)(endTime - lastTime);
		lastTime = endTime;
	}

	timeEndPeriod(1);
	glfwTerminate();

	return 0;
}
