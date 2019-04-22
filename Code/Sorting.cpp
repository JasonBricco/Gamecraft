//
// Gamecraft
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
	T pivot = items[end];
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

template <typename T>
static void Merge(T* items, T* left, int leftSize, T* right, int rightSize)
{
	int i = 0;
	int j = 0;
	int k = 0;
	
	int n1 = leftSize;
	int n2 = rightSize;
	
	while (i < n1 && j < n2)
	{
		if (left[i] < right[j])
			items[k++] = left[i++];
		else items[k++] = right[j++];
	}

	while (i < n1)
		items[k++] = left[i++];
	
	while (j < n2)
		items[k++] = right[j++];
}

template <typename T>
static void MergeSort(T* items, int size)
{
	if (size < 2) return;

	int mid = size / 2;
	int leftSize = mid, rightSize = size - mid;

	T* left = new T[leftSize];
	T* right = new T[rightSize];

	copy(items, items + mid, left);
	copy(items + mid, items + size, right);

	MergeSort(left, leftSize);
	MergeSort(right, rightSize);

	Merge(items, left, leftSize, right, rightSize);
}
