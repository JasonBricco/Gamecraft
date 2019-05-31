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
struct LockedObjectPool : public ObjectPool<T>
{
	CRITICAL_SECTION cs;

	LockedObjectPool()
	{
		InitializeCriticalSection(&cs);
	}

	T* Get()
	{
		EnterCriticalSection(&cs);
		T* item = ObjectPool::Get();
		LeaveCriticalSection(&cs);
		return item;
	}

	void Return(T* item)
	{
		EnterCriticalSection(&cs);
		ObjectPool::Return(item);
		LeaveCriticalSection(&cs);
	}
};
