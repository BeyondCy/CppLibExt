Introduction
============

This is a small library of C++11 extensions.

Current features:
- multi_array (multi dimensional array, similar to std::array)
- bit_mask (bit mask/ flags/ options wrapper class)
- range_iterator (iterator which knowns its range)
- packed_vector (<experimental>)
- flexible_stack (stack with flexible element sizes, similar to std::stack; <experimental>)

Examples
========
```cpp
/* --- multi_array --- */

// Classic C array:
int classicArray[4][10][2];
classicArray[2][5][0] = 4;

// boost::multi_array
boost::multi_array<int, 3> boostArray(boost::extents[4][10][2]);
boostArray[2][5][0] = 4;

// multi_array:
ext::multi_array<int, 4, 10, 2> multiArray;
multiArray[2] = 3; // Set a value for an entire 'slice' (i.e. multiArray[2][x][y] = 3 for all 0 <= x < 10 and 0 <= y < 2)
multiArray[2][5][0] = 4; // No overhead here in memory and speed compared to 'classicArray'


/* --- packed_vector --- */

/*
"std::vector" allows only a single type for all its elements.
"packed_vector" has a base type and allows inserting elements of any derived class.
All elements are still stored inside a single coherent memory block instead of distributed memory chunks.
This can be a good performance boost when storing lots of small different classes inside a single container.
*/

struct A
{
	virtual ~A()
	{
	}
	int x = 1;
};
struct B : public A
{
	int y = 2;
};
struct C
{
	int z = 0;
};

// This vector packs everything into a byte-aligned std::vector for optimal cache use.
// It allows different element types (or classes) if they have the same base type (here 'A').
ext::packed_vector<A> list;

list.push_back(A());
list.push_back(B());
list.push_back(B());

auto a = list.get<A>(0);
auto b = list.get<B>(1);
//auto c = list.get<C>(2); // 2 errors: 3rd element is of type 'B' and 'C' is not a derived type of 'A'.


/* --- flexible_stack --- */

enum TypeIds
{
	ID_CHAR,
	ID_INT,
	ID_FLOAT,
	ID_DOUBLE
};

flexible_stack<TypeIds> stack;

stack.push('x', ID_CHAR);
stack.push(1, ID_INT);
stack.push(2.3f, ID_FLOAT);
stack.push(4.5, ID_DOUBLE);

while (!stack.empty())
{
	switch (stack.top_id())
	{
		case ID_CHAR:
			char c = stack.top<char>();
			break;
		case ID_INT:
			int i = stack.top<int>();
			break;
		case ID_FLOAT:
			float f = stack.top<float>();
			break;
		case ID_DOUBLE:
			double d = stack.top<double>();
			break;
	}
	stack.pop();
}

```
