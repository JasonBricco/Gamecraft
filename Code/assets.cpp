//
// Jason Bricco
//

static void LoadTexture(Texture* tex, char* asset)
{
    int width, height, components;

    char* path = PathToExe(asset);
    uint8_t* data = stbi_load(path, &width, &height, &components, STBI_rgb_alpha);
    Free<char>(path);
    assert(data != nullptr);

    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void LoadTextureArray(TextureArray* tex, char** paths, bool mipMaps)
{
    int count = sb_count(paths);
    uint8_t** dataList = Malloc<uint8_t*>(count);

    int width = 0, height = 0, components;

    for (int i = 0; i < count; i++)
    {
        char* path = PathToExe(paths[i]);
        dataList[i] = stbi_load(path, &width, &height, &components, STBI_rgb_alpha);
        Free<char>(path);
        assert(dataList[i] != nullptr);
    }

    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, *tex);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 6, GL_RGBA8, width, height, count);

    for (int i = 0; i < count; i++)
    {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA,
            GL_UNSIGNED_BYTE, dataList[i]);
        stbi_image_free(dataList[i]);
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (mipMaps) glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    Free<uint8_t*>(dataList);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

static bool ShaderHasErrors(GLuint shader, ShaderType type)
{
    int status = 0;
    GLint length = 0;

    if (type == SHADER_PROGRAM)
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &status);

        if (status == GL_FALSE)
        {
            glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);

            GLchar* errorLog = Malloc<GLchar>(length);
            glGetProgramInfoLog(shader, length, NULL, errorLog);
            
            OutputDebugString("Error! Shader program failed to link.");
            OutputDebugString(errorLog);
            Free<GLchar>(errorLog);
            return true;
        }
    }
    else
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        if (status == GL_FALSE)
        {
            glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);

            GLchar* errorLog = Malloc<GLchar>(length);
            glGetShaderInfoLog(shader, length, NULL, errorLog);
            
            OutputDebugString("Error! Shader failed to compile.");
            OutputDebugString(errorLog);
            Free<GLchar>(errorLog);
            return true;
        }
    }

    return false;
}

static Shader LoadShader(char* path)
{
    char* code = ReadFileData(path);

    if (code == nullptr)
    {
        OutputDebugString("Failed to load shader from file.");
        OutputDebugString(path);
        abort();
    }

    char* vertex[2] = { "#version 440 core\n#define VERTEX 1\n", code };
    char* frag[2] = { "#version 440 core\n", code };

    GLuint vS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vS, 2, vertex, NULL);
    glCompileShader(vS);
    
    if (ShaderHasErrors(vS, VERTEX_SHADER))
    {
        OutputDebugString("Failed to compile the vertex shader.");
        abort();
    }

    GLuint fS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fS, 2, frag, NULL);
    glCompileShader(fS);
    
    if (ShaderHasErrors(fS, FRAGMENT_SHADER))
    {
        OutputDebugString("Failed to compile the fragment shader.");
        abort();
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vS);
    glAttachShader(program, fS);
    glLinkProgram(program);
    
    if (ShaderHasErrors(program, SHADER_PROGRAM))
    {
        OutputDebugString("Failed to link the shaders into the program.");
        abort();
    }

    delete[] code;
    glDeleteShader(vS);
    glDeleteShader(fS);

    return { program };
}

static void LoadAssets(Assets* assets)
{
    Shader diffuseArray = LoadShader("Shaders\\diffuse_array.shader");
    diffuseArray.view = glGetUniformLocation(diffuseArray.handle, "view");
    diffuseArray.model = glGetUniformLocation(diffuseArray.handle, "model");
    diffuseArray.proj = glGetUniformLocation(diffuseArray.handle, "projection");
    assets->diffuseArray = diffuseArray;

    Shader fluidArray = LoadShader("Shaders\\fluid_array.shader");
    fluidArray.view = glGetUniformLocation(fluidArray.handle, "view");
    fluidArray.model = glGetUniformLocation(fluidArray.handle, "model");
    fluidArray.proj = glGetUniformLocation(fluidArray.handle, "projection");
    fluidArray.time = glGetUniformLocation(fluidArray.handle, "time");
    assets->fluidArray = fluidArray;

    Shader crosshair = LoadShader("Shaders\\Crosshair.shader");
    crosshair.model = glGetUniformLocation(crosshair.handle, "model");
    crosshair.proj = glGetUniformLocation(crosshair.handle, "projection");
    assets->crosshair = crosshair;

    TextureArray blockTextures;

    char** paths = NULL;

    sb_push(paths, "Assets/Grass.png");
    sb_push(paths, "Assets/GrassSide.png");
    sb_push(paths, "Assets/Dirt.png");
    sb_push(paths, "Assets/Stone.png");
    sb_push(paths, "Assets/Water.png");
    sb_push(paths, "Assets/Sand.png");
    sb_push(paths, "Assets/Crate.png");
    sb_push(paths, "Assets/StoneBrick.png");

    LoadTextureArray(&blockTextures, paths, true);
    sb_free(paths);

    assets->blockTextures = blockTextures;

    Texture crosshairTex;
    LoadTexture(&crosshairTex, "Assets/Crosshair.png");
    assets->crosshairTex = crosshairTex;
}
