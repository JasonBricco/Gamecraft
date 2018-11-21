//
// Jason Bricco
//

static char* PathToExe(char* fileName)
{
    int size = MAX_PATH * 2;
    char* path = Malloc<char>(size);
    GetModuleFileName(0, path, size);

    char* pos = strrchr(path, '\\');
    *(pos + 1) = '\0';

    strcat(path, fileName);
    return path;
}

static char* ReadFileData(char* fileName)
{
    char* path = PathToExe(fileName);
    char* buffer = nullptr;

    ifstream file(path);
    Free<char>(path);

    if (file)
    {
        file.seekg(0, file.end);
        uint32_t length = (uint32_t)file.tellg() + 1;
        file.seekg(0, file.beg);

        char* inputBuffer = (char*)malloc(length);
        memset(inputBuffer, 0, length);
        file.read(inputBuffer, length);
        inputBuffer[length - 1] = 0;

        if (inputBuffer) file.close();
        else
        {
            OutputDebugString("Failed to read file!");
            file.close();
            free(inputBuffer);
            return nullptr;
        }

        buffer = inputBuffer;
        inputBuffer = nullptr;
    }
    else
    {
        OutputDebugString("Could not find the file: ");
        OutputDebugString(path);
        return nullptr;
    }

    return buffer;
}

static inline void WriteBinary(char* path, char* data, int length)
{
    ofstream file;
    file.open(path, ios::out | ios::binary);
    assert(file.is_open());
    file.write(data, length);
    file.close();
}

static inline void ReadBinary(char* path, char* ptr)
{
    ifstream file;
    file.open(path, ios::out | ios::binary);
    assert(file.is_open());

    file.seekg(0, file.end);
    auto length = file.tellg();
    file.seekg(0, file.beg);

    file.read(ptr, length);
    file.close();
}

static string GetLastErrorText()
{
    DWORD errorMessageID = GetLastError();
    
    if (errorMessageID == 0)
        return string("No error given.");

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    string message(messageBuffer, size);
    LocalFree(messageBuffer);

    return message;
}

#if _DEBUG
#define Print(...) { \
    char print_buffer[256]; \
    snprintf(print_buffer, sizeof(print_buffer), __VA_ARGS__); \
    OutputDebugString(print_buffer); \
}

#define Error(...) { \
    char error_buffer[256]; \
    snprintf(error_buffer, sizeof(error_buffer), __VA_ARGS__); \
    DebugBreak(); \
}
#else
#define Print(...)

#define Error(...) { \
    char error_buffer[256]; \
    snprintf(error_buffer, sizeof(error_buffer), __VA_ARGS__); \
    MessageBox(NULL, error_buffer, NULL, MB_OK); \
}
#endif
