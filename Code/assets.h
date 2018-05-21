// Voxel Engine
// Jason Bricco

typedef GLuint TextureArray;
typedef GLuint Texture;

struct Shader
{
    GLuint handle;
    GLint view, model, proj, time;
};

struct Assets
{
    TextureArray blockTextures;
    Texture crosshairTex;

    Shader diffuseArray;
    Shader fluidArray;
    Shader crosshair;
};
