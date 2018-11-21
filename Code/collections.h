//
// Jason Bricco
//

template <typename T>
class Stack
{
	T* items;
	int count, max;

public:
	Stack(int capacity)
	{
		items = Malloc<T>(capacity);
		count = 0;
		max = capacity;
	}

	inline int Count()
	{
		return count;
	}

	inline T Pop()
	{
		assert(count > 0);
		return items[--count];
	}

	inline void Push(T item)
	{
		if (count + 1 > max)
		{
			max *= 2;
			items = Realloc<T>(items, max);
		}

		items[count++] = item;
	}

	inline void Clear()
	{
		count = 0;
	}

	~Stack()
	{
		Free<T>(items);
	}
};
