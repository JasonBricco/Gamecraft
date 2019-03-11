//
// Jason Bricco
//

template <typename T>
static int BinarySearch(T* items, int size, T key)
{
	int left = 0, right = size - 1;

	while (left <= right)
	{
		int mid = (left + right) / 2;
		T item = items[mid];

		if (item == key)
			return mid;
		else if (key < item)
			right = mid - 1;
		else left = mid + 1;
	}

	return -1;
}

template <typename T>
static void BubbleSort(T* items, int size)
{
	for (int i = 0; i < size - 1; i++)
	{
		bool swapped = false;

		for (int j = size - 1; j > i; j--)
		{
			if (items[j] < items[j - 1])
			{
				T temp = items[j];
				items[j] = items[j - 1];
				items[j - 1] = temp;
				swapped = true;
			}
		}

		if (!swapped)
			break;
	}
}

template <typename T>
static void InsertionSort(T* items, int size)
{
	for (int i = 1; i < size; i++)
	{
		T cur = items[i];
		int j = i - 1;

		while (j >= 0 && items[j] > cur)
		{
			items[j + 1] = items[j];
			j--;
		}

		items[j + 1] = cur;
	}
}

template <typename T>
static void SelectionSort(T* items, int size)
{
	for (int i = 0; i < size - 1; i++)
	{
		int min = i;

		for (int j = i + 1; j < size; j++)
		{
			if (items[j] < items[min])
				min = j;
		}

		if (min != i)
		{
			T temp = items[i];
			items[i] = items[min];
			items[min] = temp;
		}
	}
}

template <typename T>
static inline void SwapInArray(T* items, int a, int b)
{
	T temp = items[a];
	items[a] = items[b];
	items[b] = temp;
}

template <typename T>
static int QuickSortPartition(T* items, int start, int end)
{
	int pivot = items[end];
	int left = start;
	int right = end - 1;

	while (left <= right)
	{
		while (left <= right && items[left] <= pivot)
			left++;

		while (left <= right && items[right] >= pivot)
			right--;

		if (left < right)
		{
			SwapInArray(items, left, right);
			left++;
			right--;
		}
	}

	SwapInArray(items, left, end);
	return left;
}

template <typename T>
static void QuickSort(T* items, int start, int end)
{
	if (start < end)
	{
		int pivot = QuickSortPartition(items, start, end);
		QuickSort(items, start, pivot - 1);
		QuickSort(items, pivot + 1, end);
	}
}

template <typename K, typename V>
struct PQEntry
{
	K key;
	V value;
};

template <typename K, typename V>
struct PriorityQueue
{
	vector<PQEntry<K, V>> entries;

	int Parent(int i)
	{
		return (i - 1) / 2;
	}

	int Left(int i)
	{
		return 2 * i + 1;
	}

	int Right(int i)
	{
		return 2 * i + 2;
	}

	bool HasLeft(int i)
	{
		return Left(i) < entries.size();
	}

	bool HasRight(int i)
	{
		return Right(i) < entries.size();
	}

	void Swap(int a, int b)
	{
		PQEntry<K, V> temp = entries[a];
		entries[a] = entries[b];
		entries[b] = temp;
	}

	void UpHeap(int i)
	{
		if (i == 0) return;

		int p = Parent(i);

		if (entries[i].key < entries[p].key)
		{
			Swap(i, p);
			UpHeap(p);
		}
	}

	void DownHeap(int i)
	{
		if (!HasLeft(i))
			return;

		int s = Left(i);

		if (HasRight(i))
		{
			int r = Right(i);

			if (entries[r].key < entries[s].key) 
				s = r;
		}

		if (entries[s].key < entries[i].key)
		{
			Swap(i, s);
			DownHeap(s);
		}
	}

	PQEntry<K, V> Insert(K key, V value)
	{
		PQEntry<K, V> entry = { key, value };
		entries.push_back(entry);
		UpHeap((int)entries.size() - 1);
		return entry;
	}

	PQEntry<K, V> RemoveMin()
	{
		PQEntry<K, V> root = entries[0];
		entries[0] = entries[entries.size() - 1];
		entries.pop_back();
		DownHeap(0);
		return root;
	}
};

#if TESTING

static void TestBinarySearch()
{
	int items[5] = { 1, 2, 3, 4, 5 };
	AssertEquals(BinarySearch(items, ArrayLength(items), 3), 2);
	AssertEquals(BinarySearch(items, ArrayLength(items), 1), 0);
	AssertEquals(BinarySearch(items, ArrayLength(items), 5), 4);
	AssertEquals(BinarySearch(items, ArrayLength(items), 6), -1);
}

static void TestBubbleSort()
{
	int items[5] = { 5, 4, 3, 2, 1 };
	int expected[5] = { 1, 2, 3, 4, 5 };
	BubbleSort(items, ArrayLength(items));
	AssertArrayEquals(items, expected, ArrayLength(items));

	int items2[5] = { 2, 5, 1, 7, 5 };
	int expected2[5] = { 1, 2, 5, 5, 7 };
	BubbleSort(items2, ArrayLength(items2));
	AssertArrayEquals(items2, expected2, ArrayLength(items2));
}

static void TestInsertionSort()
{
	int items[5] = { 5, 4, 3, 2, 1 };
	int expected[5] = { 1, 2, 3, 4, 5 };
	InsertionSort(items, ArrayLength(items));
	AssertArrayEquals(items, expected, ArrayLength(items));

	int items2[5] = { 2, 5, 1, 7, 5 };
	int expected2[5] = { 1, 2, 5, 5, 7 };
	InsertionSort(items2, ArrayLength(items2));
	AssertArrayEquals(items2, expected2, ArrayLength(items2));
}

static void TestSelectionSort()
{
	int items[5] = { 5, 4, 3, 2, 1 };
	int expected[5] = { 1, 2, 3, 4, 5 };
	SelectionSort(items, ArrayLength(items));
	AssertArrayEquals(items, expected, ArrayLength(items));

	int items2[5] = { 2, 5, 1, 7, 5 };
	int expected2[5] = { 1, 2, 5, 5, 7 };
	SelectionSort(items2, ArrayLength(items2));
	AssertArrayEquals(items2, expected2, ArrayLength(items2));
}

static void TestPQSort()
{
	int items[5] = { 5, 4, 3, 2, 1 };
	int expected[5] = { 1, 2, 3, 4, 5 };

	PriorityQueue<int, int> queue;

	for (int i = 0; i < ArrayLength(items); i++)
		queue.Insert(items[i], -1);

	for (int i = 0; i < ArrayLength(items); i++)
		items[i] = queue.RemoveMin().key;

	AssertArrayEquals(items, expected, ArrayLength(items));
}

static void TestQuickSort()
{
	int items[10] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
	int expected[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	QuickSort(items, 0, ArrayLength(items) - 1);
	AssertArrayEquals(items, expected, ArrayLength(items));

	int items2[10] = { 9, 3, 5, 1, 4, 10, 2, 8, 6, 7 };
	int expected2[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	QuickSort(items2, 0, ArrayLength(items2) - 1);
	AssertArrayEquals(items2, expected2, ArrayLength(items2));
}

static void TestGenericAlgorithms()
{
	TestBinarySearch();
	TestBubbleSort();
	TestInsertionSort();
	TestSelectionSort();
	TestPQSort();
	TestQuickSort();
}

#endif
