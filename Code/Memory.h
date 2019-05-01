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
