//
// Gamecraft
//

template <typename T>
struct List
{
	int size, _capacity;
	T* items;

	void Reserve(int capacity)
	{
		items = (T*)malloc(capacity * sizeof(T));
		_capacity = capacity;
	}

	bool Empty()
	{
		return size == 0;
	}

	bool IndexValid(int i, int n)
	{
		return i >= 0 && i < n;
	}

	T& operator[](int i)
	{
		assert(IndexValid(i, size));
		return items[i];
	}


	void Add(T item)
	{
		if (size + 1 > _capacity)
		{
			_capacity *= 2;
			items = (T*)realloc(items, _capacity * sizeof(T));
		}

		items[size++] = item;
	}

	void Clear()
	{
		size = 0;
	}
};
