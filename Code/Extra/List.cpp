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
		items = (T*)malloc(capacity * sizeof(T));
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

	void GrowIfNeeded()
	{
		if (size == capacity)
		{
			capacity *= 2;
			items = (T*)realloc(items, capacity * sizeof(T));
		}
	}

	void Add(int i, T item)
	{
		GrowIfNeeded();

		// Shift all items at the given index and further one position to the right.
		for (int j = size - 1; j >= i; j--)
			items[j + 1] = items[j];

		items[i] = item;
		size++;
	}

	T Remove(int i)
	{
		assert(IndexValid(i, size));
		T item = items[i];

		// Shift all elements from the index and further one position to the left.
		for (int j = i; j < size; j++)
			items[j] = items[j + 1];

		size--;
		return item;
	}

	void AddFirst(T item)
	{
		Add(0, item);
	}

	void AddLast(T item)
	{
		Add(size, item);
	}

	T RemoveFirst()
	{
		assert(!IsEmpty());
		return Remove(0);
	}

	T RemoveLast()
	{
		assert(!IsEmpty());
		return Remove(size - 1);
	}
};
