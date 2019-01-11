//
// Jason Bricco
//

struct GameState;
struct Camera;

struct UI
{
	GLFWcursor* cursors[ImGuiMouseCursor_COUNT];
	bool mouseJustPressed[2];
	GLuint fontTexture, va, vb, ib;
};

static void RenderUI(GameState* state, Camera* cam, UI& ui);