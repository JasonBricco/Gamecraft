//
// Gamecraft
//

template <typename T>
struct List
{
	int size, capacity;
	T* items;

	List(int capacity = 16)
	{
		items = AllocArray(capacity, T);
		size = 0;
		this->capacity = capacity;
	}

	bool IsEmpty()
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

	T Set(int i, T item)
	{
		assert(IndexValid(i, size));
		T prev = items[i];
		items[i] = item;
		return prev;
	}

	void Add(int i, T item)
	{
		assert(size < capacity);

		for (int j = size - 1; j >= i; j--)
			items[j + 1] = items[j];

		items[i] = item;
		size++;
	}

	T Remove(int i)
	{
		assert(IndexValid(i, size));
		T item = items[i];

		for (int j = i; j < size; j++)
			items[j] = items[j + 1];

		size--;
		return item;
	}

	T RemoveSwapWithLast(int i)
	{
		assert(IndexValid(i, size));
		T item = items[size - 1];
		items[i] = item;
		size--;
		return item;
	}

	void AddFirst(T item)
	{
		Add(0, item);
	}

	void AddLast(T item)
	{
		assert(size < capacity);
		items[size++] = item;
	}

	void AddLastSafe(T item)
	{
		if (size < capacity)
			items[size++] = item;
	}

	T RemoveFirst()
	{
		assert(!IsEmpty());
		return Remove(0);
	}

	T RemoveLast()
	{
		assert(!IsEmpty());
		return items[--size];
	}

	void Clear()
	{
		size = 0;
	}

	T* Begin()
	{
		return items;
	}

	T* End()
	{
		return items + size;
	}
};

template <typename T>
struct Queue
{
	T* items;
	int read, write;
	int size, capacity;

	Queue(int capacity = 16)
	{
		items = AllocArray(capacity, T);
		read = 0;
		write = 0;
		size = 0;
		this->capacity = capacity;
	}

	bool IsEmpty()
	{
		return size == 0;
	}

	void Enqueue(T item)
	{
		assert(size < capacity);
		items[write] = item;
		write = (write + 1) % capacity;
		size++;
	}

	T Peek()
	{
		assert(!IsEmpty());
		return items[read];
	}

	T Dequeue()
	{
		assert(!IsEmpty());
		T item = items[read];
		read = (read + 1) % capacity;
		size--;
		return item;
	}
};
