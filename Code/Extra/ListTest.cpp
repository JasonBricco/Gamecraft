//
// Gamecraft
//

static void ListAddGetTest()
{
	List<int> list = List<int>();

	AssertEquals(list.capacity, 16);
	list.Add(0, 0);
	AssertEquals(list[0], 0);
	list.Add(1, 1);
	list.Add(2, 2);
	AssertEquals(list.size, 3);
	AssertEquals(list[2], 2);
	list.Add(0, 26);
	AssertEquals(list.size, 4);
	AssertEquals(list[0], 26);
	AssertEquals(list[1], 0);
	AssertEquals(list[3], 2);
	list.Add(list.size, 24);
	AssertEquals(list[list.size - 1], 24);
}

static void ListSetTest()
{
	List<int> list = List<int>();

	list.Add(0, 5);
	int v = list.Set(0, 3);
	AssertEquals(v, 5);
	AssertEquals(list[0], 3);
}

static void ListAddLastTest() 
{
	List<int> list = List<int>();

	for (int i = 1; i <= 50; i++)
		list.AddLast(i);

	AssertEquals(list.size, 50);
	
	for (int i = 1; i <= 50; i++)
		AssertEquals(list[i - 1], i);
}

static void ListAddFirstTest() 
{
	List<int> list = List<int>();

	for (int i = 1; i <= 50; i++)
		list.AddFirst(i);
	
	AssertEquals(list.size, 50);
	
	int i = 0;

	for (int l = 50; l >= 1; l--)
		AssertEquals(list[i++], l);
}

static void ListRemoveTest()
{
	List<int> list = List<int>();

	for (int i = 1; i <= 10; i++)
		list.AddLast(i);
	
	list.Remove(5);
	
	AssertEquals(list.size, 9);
	
	for (int i = 0; i <= 4; i++)
		AssertEquals(list[i], i + 1);

	for (int i = 5; i <= 8; i++)
		AssertEquals(list[i], i + 2);
}

static void ListRemoveFirstLastTest()
{
	List<int> list = List<int>();

	for (int i = 0; i < 10; i++)
		list.AddLast(i);
	
	int v = list.RemoveFirst();
	AssertEquals(v, 0);
	AssertEquals(list[0], 1);
	
	v = list.RemoveLast();
	AssertEquals(v, 9);
	AssertEquals(list[list.size - 1], 8);
}

static void TestList()
{
	ListAddGetTest();
	ListSetTest();
	ListAddLastTest();
	ListAddFirstTest();
	ListRemoveTest();
	ListRemoveFirstLastTest();
}
