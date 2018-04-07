// Voxel Engine
// Jason Bricco

#define PROFILING 1
#define ASSERTIONS 1
#define DEBUG_MEMORY 0

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLEW_STATIC
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <TimeAPI.h>
#include <time.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "FastNoiseSIMD.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_ONLY_PNG

#include "stb_image.h"
#include "stretchy_buffer.h"

#include <unordered_map>
#include <fstream>

#include "glm/fwd.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#if DEBUG_MEMORY
#define _CRTDBG_MAP_ALLOC 1
#include <crtdbg.h>  
#endif

using namespace glm;
using namespace std;

#if ASSERTIONS
#define Assert(expression) if (!(expression)) { abort(); }
#else
#define Assert(expression)
#endif

#define Malloc(type) (type*)malloc(sizeof(type))
#define Calloc(type) (type*)calloc(1, sizeof(type))

inline void DisplayError(char* message)
{
	fprintf(stderr, "%s\n", message);
}

static char* PathToAsset(char* fileName)
{
	int size = MAX_PATH * 2;
	char* path = (char*)malloc(size);
	GetModuleFileName(NULL, path, size);

	char* pos = strrchr(path, '\\');
	*(pos + 1) = '\0';

	strcat(path, fileName);
	return path;
}

#include "utils.h"
#include "renderer.h"
#include "world.h"
#include "collision.h"
#include "entity.h"

#include "renderer.cpp"
#include "collision.cpp"
#include "world.cpp"
#include "entity.cpp"

static bool g_shiftHeld;
static bool g_paused = false;

static void OnKey(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		g_paused = true;
	}

	if (mode == GLFW_MOD_SHIFT) g_shiftHeld = true;
	else g_shiftHeld = false;
}

static void OnMouseMove(GLFWwindow* window, double posX, double posY)
{
	OrbitCamera* cam = ((Renderer*)glfwGetWindowUserPointer(window))->orbit;

	if (cam != NULL)
	{
		static vec2 lastMouse = vec2(0.0f, 0.0f);

		float x = (float)posX, y = (float)posY;

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == 1)
		{
			cam->yaw -= (x - lastMouse.x) * cam->sensitivity;
			cam->pitch += (y - lastMouse.y) * cam->sensitivity;
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == 1)
		{
			float dX = 0.01f * (x - lastMouse.x);
			float dY = 0.01f * (y - lastMouse.y);
			SetCameraRadius(cam, cam->radius + (dX - dY));
		}

		lastMouse.x = x;
		lastMouse.y = y;
	}
}

static void OnMouseButton(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
    	if (g_paused)
    	{
    		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    		g_paused = false;
    	}
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
	ivec3 pos = ToChunkPos(player->pos);

	UnloadChunks(world, pos);
	LoadSurroundingChunks(world, pos);

	if (g_paused) return;

	double mouseX, mouseY;

	glfwGetCursorPos(window, &mouseX, &mouseY);

	double cX = g_windowWidth / 2.0, cY = g_windowHeight / 2.0f;

	Camera* cam = player->camera;

	float rotX = (float)(cX - mouseX) * cam->sensitivity;
	float rotY = (float)(cY - mouseY) * cam->sensitivity;

	RotateCamera(cam, rotX, rotY);

	glfwSetCursorPos(window, cX, cY);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
		Move(world, player, player->speed * deltaTime * cam->look);

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) 
		Move(world, player, player->speed * deltaTime * -cam->look);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) 
		Move(world, player, player->speed * deltaTime * -cam->right);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) 
		Move(world, player, player->speed * deltaTime * cam->right);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) 
		Move(world, player, player->speed * deltaTime * g_worldUp);

	if (g_shiftHeld) Move(world, player, player->speed * deltaTime * -g_worldUp);
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
	glfwSetCursorPosCallback(window, OnMouseMove);
	glfwSetMouseButtonCallback(window, OnMouseButton);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, g_windowWidth / 2.0f, g_windowHeight / 2.0f);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* vMode = glfwGetVideoMode(monitor);

	Renderer* renderer = Calloc(Renderer);
	glfwSetWindowUserPointer(window, renderer);
	
	GLuint program = LoadShaders("Shaders\\diffusevertarray.shader", "Shaders\\diffusefragarray.shader");
	
	TextureArray texture;

	char** paths = NULL;
	sb_push(paths, PathToAsset("Assets/Grass.png"));
	sb_push(paths, PathToAsset("Assets/GrassSide.png"));
	sb_push(paths, PathToAsset("Assets/Dirt.png"));

	LoadTextureArray(&texture, paths, true);
	sb_free(paths);

	World* world = NewWorld(10, 10);

	Player* player = NewPlayer(world->spawn);
	Camera* cam = player->camera;
	renderer->cam = cam;

	mat4 projection = perspective(radians(CAMERA_FOV), (float)g_windowWidth / (float)g_windowHeight, 0.1f, 1000.0f);
	
	int refreshHz = vMode->refreshRate * 2;
	float targetSeconds = 1.0f / refreshHz;
	double lastTime = glfwGetTime();

	float deltaTime = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		Update(window, player, world, deltaTime);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 view = GetViewMatrix(cam);

		UseShader(program);
		SetUniform(program, "view", view);
		SetUniform(program, "projection", projection);
		SetUniform(program, "viewPos", cam->pos);
		SetUniform(program, "ambient", vec4(1.0f, 1.0f, 1.0f, 1.0f));

		mat4 model;

		for (int i = 0; i < world->loadedCount; i++)
		{
			Chunk* next = world->loadedChunks[i];

   			if (next->state == CHUNK_BUILT)
			{
				model = translate(mat4(), (vec3)next->wPos);
				SetUniform(program, "model", model);
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
