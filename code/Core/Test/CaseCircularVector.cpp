#include "Core/Containers/CircularVector.h"
#include "Core/Test/CaseCircularVector.h"

namespace traktor::test
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseCircularVector", 0, CaseCircularVector, Case)

void CaseCircularVector::run()
{

	CircularVector< int32_t, 4 > v;

	v.push_back(1);
	CASE_ASSERT_EQUAL(v.size(), 1);
	CASE_ASSERT(!v.full());

	v.push_back(2);
	CASE_ASSERT_EQUAL(v.size(), 2);
	CASE_ASSERT(!v.full());

	v.push_back(3);
	CASE_ASSERT_EQUAL(v.size(), 3);
	CASE_ASSERT(!v.full());

	v.push_back(4);
	CASE_ASSERT_EQUAL(v.size(), 4);
	CASE_ASSERT(v.full());

	CASE_ASSERT_EQUAL(v[0], 1);
	CASE_ASSERT_EQUAL(v[1], 2);
	CASE_ASSERT_EQUAL(v[2], 3);
	CASE_ASSERT_EQUAL(v[3], 4);

	v.push_back(5);
	CASE_ASSERT_EQUAL(v.size(), 4);
	CASE_ASSERT(v.full());

	CASE_ASSERT_EQUAL(v[0], 2);
	CASE_ASSERT_EQUAL(v[1], 3);
	CASE_ASSERT_EQUAL(v[2], 4);
	CASE_ASSERT_EQUAL(v[3], 5);

	v.pop_front();
	CASE_ASSERT_EQUAL(v.size(), 3);
	CASE_ASSERT(!v.full());

	CASE_ASSERT_EQUAL(v[0], 3);
	CASE_ASSERT_EQUAL(v[1], 4);
	CASE_ASSERT_EQUAL(v[2], 5);

	v.pop_front();
	CASE_ASSERT_EQUAL(v.size(), 2);
	CASE_ASSERT(!v.full());

	CASE_ASSERT_EQUAL(v[0], 4);
	CASE_ASSERT_EQUAL(v[1], 5);

	v.push_back(6);
	CASE_ASSERT_EQUAL(v.size(), 3);
	CASE_ASSERT(!v.full());

	v.push_back(7);
	CASE_ASSERT_EQUAL(v.size(), 4);
	CASE_ASSERT(v.full());

	CASE_ASSERT_EQUAL(v[0], 4);
	CASE_ASSERT_EQUAL(v[1], 5);
	CASE_ASSERT_EQUAL(v[2], 6);
	CASE_ASSERT_EQUAL(v[3], 7);	
}

}
