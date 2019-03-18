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

// Updates the simplex for the three point (triangle) case. 'abc' must be in 
// counterclockwise winding order.
static void UpdateSimplex3(vec3& a, vec3& b, vec3& c, vec3& d, int& dim, vec3& search)
{
	// Triangle normal.
	vec3 norm = cross(b - a, c - a);

	// Direction to the origin.
	vec3 ao = -a;

	// Determine which feature of the triangle is closest to the origin and make it
	// the new simplex - any of its edges, or in front of or behind it. 
	dim = 2;

	// Origin is closest to edge 'ab'.
	if (dot(cross(b - a, norm), ao) > 0)
	{
		c = a;
		search = cross(cross(b - a, ao), b - a);
		return;
	}

	// Origin is closest to edge 'ac'.
	if (dot(cross(norm, c - a), ao) > 0)
	{
		b = a;
		search = cross(cross(c - a, ao), c - a);
		return;
	}

	dim = 3;

	// Origin is above the triangle.
	if (dot(norm, ao) > 0)
	{
		d = c;
		c = b;
		b = a;
		search = norm;
		return;
	}

	// Origin is below the triangle.
	d = b;
	b = a;
	search = -norm;
}

// Updates the simplex for the four point (tetrahedron) case. 'a' is the top of the 
// tetrahedron. 'bcd' is the base in counterclockwise winding order. We know the 
// origin is above 'bcd' and below 'a' before calling.
static bool UpdateSimplex4(vec3& a, vec3& b, vec3& c, vec3& d, int& dim, vec3& search)
{
	// Normals of the three non-base tetrahedron faces.
	vec3 abc = cross(b - a, c - a);
	vec3 acd = cross(c - a, d - a);
	vec3 adb = cross(d - a, b - a);

	vec3 ao = -a;
	dim = 3;

	// Origin is in front of 'abc'.
	if (dot(abc, ao) > 0)
	{
		d = c;
		c = b;
		b = a;
		search = abc;
		return false;
	}

	// Origin is in front of 'acd'.
	if (dot(acd, ao) > 0)
	{
		b = a;
		search = acd;
		return false;
	}

	// Origin is in front of 'adb'.
	if (dot(adb, ao) > 0)
	{
		c = d;
		d = b;
		b = a;
		search = adb;
		return false;
	}

	return true;
}

struct Collider
{
	vec3 pos;
    virtual vec3 Support(vec3 dir) = 0;
};

// Expanding polytope algorithm for finding the minimum translation vector.
static tuple<vec3, vec3> EPA(vec3 a, vec3 b, vec3 c, vec3 d, Collider* colA, Collider* colB)
{
	// Each triangle face has three vertices and a normal.
	vec3 faces[64][4];

	// Begin the array with the final simplex from GJK.
	faces[0][0] = a;
	faces[0][1] = b;
	faces[0][2] = c;
	faces[0][3] = normalize(cross(b - a, c - a)); 

	faces[1][0] = a;
	faces[1][1] = c;
	faces[1][2] = d;
	faces[1][3] = normalize(cross(c - a, d - a));

	faces[2][0] = a;
	faces[2][1] = d;
	faces[2][2] = b;
	faces[2][3] = normalize(cross(d - a, b - a));

	faces[3][0] = b;
	faces[3][1] = d;
	faces[3][2] = c;
	faces[3][3] = normalize(cross(d - b, c - b));

	int faceCount = 4;
	int closest;

	for (int iter = 0; iter < 32; iter++)
	{
		// Find the face that's closest to the origin.
		float minDist = dot(faces[0][0], faces[0][3]);
		closest = 0;

		for (int i = 1; i < faceCount; i++)
		{
			float dist = dot(faces[i][0], faces[i][3]);

			if (dist < minDist)
			{
				minDist = dist;
				closest = i;
			}
		}

		// Normal of the face closest to the origin.
		vec3 search = faces[closest][3];

		vec3 p = colB->Support(search) - colA->Support(-search);

		// dot product between the vertex and normal gives the resolution of the collision along the normal. 
		if (dot(p, search) - minDist < EPA_TOLERANCE)
		{
			vec3 mtv = faces[closest][3] * dot(p, search); 
			return { mtv, faces[closest][3] };
		}

		// Tracks edges that must be fixed after removing faces.
		vec3 looseEdges[32][2];
		int looseCount = 0;

		// Find all triangles facing point p.
		for (int i = 0; i < faceCount; i++)
		{
			// If triangle i faces p, remove it.
			if (dot(faces[i][3], p - faces[i][0]) > 0)
			{
				// Add removed triangle's edges to loose edge list but remove it if it's already there.
				for (int j = 0; j < 3; j++)
				{
					vec3 currentEdge[2] = { faces[i][j], faces[i][(j + 1) % 3] };
					bool found = false;

					// Checks to see if the current edge is already in the list.
					for (int k = 0; k < looseCount; k++)
					{
						if (looseEdges[k][1] == currentEdge[0] && looseEdges[k][0] == currentEdge[1])
						{
							// Edge is already in the list, remove it. 
							looseEdges[k][0] = looseEdges[looseCount - 1][0];
							looseEdges[k][1] = looseEdges[looseCount - 1][1];
							looseCount--;
							found = true;

							// Exit loop as the edge can only be shared once.
							k = looseCount; 
						}
					}

					if (!found)
					{
						// Add current edge to the list.
						if (looseCount > 32) break;

						looseEdges[looseCount][0] = currentEdge[0];
						looseEdges[looseCount][1] = currentEdge[1];
						looseCount++;
					}
				}

				faces[i][0] = faces[faceCount - 1][0];
				faces[i][1] = faces[faceCount - 1][1];
				faces[i][2] = faces[faceCount - 1][2];
				faces[i][3] = faces[faceCount - 1][3];
				faceCount--;
				i--;
			}
		}

		// Reconstruct the polytope with point p added.
		for (int i = 0; i < looseCount; i++)
		{
			if (faceCount > 64) break;

			faces[faceCount][0] = looseEdges[i][0];
			faces[faceCount][1] = looseEdges[i][1];
			faces[faceCount][2] = p;
			faces[faceCount][3] = normalize(cross(looseEdges[i][0] - looseEdges[i][1], looseEdges[i][0] - p));

			// Check for the wrong normal to maintain counterclockwise winding.
			float bias = 0.00001f;

			if (dot(faces[faceCount][0], faces[faceCount][3]) + bias < 0)
			{
				vec3 temp = faces[faceCount][0];
				faces[faceCount][0] = faces[faceCount][1];
				faces[faceCount][1] = temp;
				faces[faceCount][3] = -faces[faceCount][3];
			}

			faceCount++;
		}
	}

	vec3 mtv = faces[closest][3] * dot(faces[closest][0], faces[closest][3]);
	return make_tuple(mtv, faces[closest][3]);
}

// Returns true if two colliders are intersecting using the GJK algorithm. 
// 'info', if given, will return a minimum translation vector and collision 
// normal using EPA.
static bool GJK(Collider* colA, Collider* colB, tuple<vec3, vec3>* info)
{
	vec3 a, b, c, d;
	vec3 search = colA->pos - colB->pos;

	// Initial simplex point.
	c = colB->Support(search) - colA->Support(-search);

	// Search in the direction of the origin.
	search = -c;

	// Second point to form a line segment of the simplex.
	b = colB->Support(search) - colA->Support(-search);

	// We haven't reached the origin, so we can't enclose it.
	if (dot(b, search) < 0) return false;

	// Search perpendicular to the line segment, toward the origin.
	search = cross(cross(c - b, -b), c - b);

	// Origin is on the line segment we created.
	if (search == vec3(0.0f))
	{
		// Set search to an arbitrary normal vector. In this case, use the x-axis.
		search = cross(c - b, vec3(1.0f, 0.0f, 0.0f));

		// If we're still on the line segment, normal with the z-axis.
		if (search == vec3(0.0f))
			search = cross(c - b, vec3(0.0f, 0.0f, -1.0f)); 
	}

	// Number of simplex dimensions.
	int dim = 2;

	for (int iter = 0; iter < 32; iter++)
	{
		a = colB->Support(search) - colA->Support(-search);

		// We cannot enclose the origin as we haven't reached it.
		if (dot(a, search) < 0) return false;

		dim++;

		if (dim == 3) UpdateSimplex3(a, b, c, d, dim, search);
		else
		{
			if (UpdateSimplex4(a, b, c, d, dim, search))
			{
				if (info != nullptr)
					*info = EPA(a, b, c, d, colA, colB);
				
				return true;
			}
		}
	}

	return false;
}
