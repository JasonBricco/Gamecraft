//
// Gamecraft
//

template <typename T>
struct ObjectPool
{
	queue<T*> items;

	T* Get()
	{
		T* item;

		if (items.empty())
			item = new T();
		else
		{
			item = items.front();
			items.pop();
		}
		
		return item;
	}
	
	void Return(T* item)
	{
		items.push(item);
	}
};

template <typename T>
struct FixedObjectPool
{
	int _max;
	vector<T*> items;

	void Create(int max)
	{
		_max = max;
		items.reserve(max);

		for (int i = 0; i < max; i++)
			items.push_back(new T());
	}

	T* Get()
	{
		if (items.size() > 0)
		{
			T* item = items.back();
			items.pop_back();
			return item;
		}

		return nullptr;
	}

	bool CanGet(int count)
	{
		return items.size() >= count;
	}
	
	void Return(T* item)
	{
		items.push_back(item);
		assert(items.size() <= _max);
	}
};
