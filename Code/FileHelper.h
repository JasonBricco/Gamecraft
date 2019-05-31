//
// Gamecraft
//

static void* ReadFileData(char* path, uint32_t* sizePtr)
{
    HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    if (file != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER sizeValue;

        if (!GetFileSizeEx(file, &sizeValue))
            return nullptr;

        uint32_t size = (uint32_t)sizeValue.QuadPart;
        void* data = malloc(size);
        
        DWORD bytesRead;

        if (ReadFile(file, data, size, &bytesRead, 0) && (size == bytesRead))
            *sizePtr = size;
        else
        {
            free(data);
            data = nullptr;
        }

        CloseHandle(file);
        return data;
    }

    return nullptr;
}

#ifndef ASSET_BUILDER

static char* PathToExe(char* fileName, char* buffer, int size)
{
    GetModuleFileName(0, buffer, size);

    char* pos = strrchr(buffer, '\\');
    *(pos + 1) = '\0';

    strcat(buffer, fileName);
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

static void DeleteDirectory(char* path)
{
    char searchPath[MAX_PATH];
    sprintf(searchPath, "%s\\*.txt", path);

    WIN32_FIND_DATA findData;
    HANDLE handle = FindFirstFile(searchPath, &findData);

    while (handle != INVALID_HANDLE_VALUE)
    {
        char filePath[MAX_PATH];
        sprintf(filePath, "%s\\%s", path, findData.cFileName);

        DeleteFile(filePath);

        if (!FindNextFile(handle, &findData))
            break;
    }

    FindClose(handle);
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

#endif
