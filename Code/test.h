//
// Jason Bricco
//

template <typename T>
static void TestFailed(T value, T expected, char* file, int line)
{
	string str = string("Expected: ") + to_string(expected) + ", actual: " + to_string(value) + " at file " + file + ", line " + to_string(line);
	char error_buffer[256];
	snprintf(error_buffer, sizeof(error_buffer), "%s\n", str.c_str());
	MessageBox(NULL, error_buffer, NULL, MB_OK | MB_ICONERROR);
	exit(-1);
}

template <typename T>
static inline void _AssertEquals(T value, T expected, char* file, int line)
{
	if (value != expected)
		TestFailed(value, expected, file, line);
}

#define AssertEquals(value, expected) _AssertEquals(value, expected, __FILE__, __LINE__)

template <typename T>
static inline void _AssertArrayEquals(T* value, T* expected, int size, char* file, int line)
{
	for (int i = 0; i < size; i++)
		_AssertEquals(value[i], expected[i], file, line);
}

#define AssertArrayEquals(value, expected, size) _AssertArrayEquals(value, expected, size, __FILE__, __LINE__)


