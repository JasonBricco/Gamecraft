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

template <typename T>
struct PriorityQueue
{
	vector<T> items;

	int Parent(int i)
	{
		return (i - 1) / 2;
	}

	int LeftChild(int i)
	{
		return 2 * i + 1;
	}

	int RightChild(int i)
	{
		return 2 * i + 2;
	}

	bool HasLeftChild(int i)
	{
		return LeftChild(i) < items.size();
	}

	bool HasRightChild(int i)
	{
		return RightChild(i) < items.size();
	}

	T HighestPriority()
	{
		assert(items.size() > 0);
		return items[0];
	}

	void UpHeap(int index)
	{
		if (index == 0)
			return;

		int p = Parent(index);

		if (items[index] > items[p])
		{
			swap(items[index], items[p]);
			Upheap(p);
		}
	}

	void DownHeap(int index)
	{
		if (!HasLeftChild(index))
			return;

		int s = LeftChild(index);

		if (HasRightChild(index))
		{
			int r = RightChild(index);

			if (items[r] > items[s]) 
				s = r;
		}

		if (items[s] > items[index])
		{
			swap(items[s], items[index]);
			DownHeap(s);
		}
	}

	void Insert(T item)
	{
		items.push_back(item);
		UpHeap(items.size() - 1);
	}

	void Remove(T item)
	{
		auto it = items.find(items.begin(), items.end(), item);
		int index = distance(items.begin(), it);

		if (it != items.end())
		{
			swap(items[index], items[items.size() - 1]);
			items.pop_back();
			DownHeap(index);
		}
	}

	bool Exists(T item)
	{
		auto it = items.find(items.begin(), items.end(), item);
		return it != items.end();
	}
};
