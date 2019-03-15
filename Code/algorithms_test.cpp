//
// Jason Bricco
//

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
