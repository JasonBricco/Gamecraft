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

template <typename T>
struct Queue
{
	T* items;
	int read, write;
	int size, capacity;

	Queue(int capacity)
	{
		assert(IsPowerOf2(capacity));
		items = new T[capacity];
		size = 0;
		read = 0;
		write = 0;
		this->capacity = capacity;
	}

	T Dequeue()
	{
		assert(size > 0);
		T item = items[read];
		read = (read + 1) & (capacity - 1);
		size--;
		return item;
	}

	void Enqueue(T item)
	{
		items[write] = item;
		write = (write + 1) & (capacity - 1);
		size++;
		assert(read != write);
	}

	bool Empty()
	{
		return size == 0;
	}

	~Queue()
	{
		delete[] items;
	}
};
