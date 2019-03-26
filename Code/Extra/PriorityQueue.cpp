//
// Gamecraft
//

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
