/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/BitReader.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Test/CaseAlignedVector.h"

namespace traktor::test
{
	namespace
	{

int32_t g_countCopy = 0;
int32_t g_countMove = 0;

class TestItem
{
public:
	int value;

	TestItem() : value(-1) {}

	TestItem(int v) : value(v) {}

	TestItem(const TestItem& s) : value(s.value) { g_countCopy++; }

	TestItem(TestItem&& s) {  value = s.value; s.value = -1; g_countMove++; }

	~TestItem() {}

	TestItem& operator = (const TestItem& s) { value = s.value; g_countCopy++; return *this; }

	TestItem& operator = (TestItem&& s) { value = s.value; s.value = -1; g_countMove++; return *this; }
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseAlignedVector", 0, CaseAlignedVector, Case)

void CaseAlignedVector::run()
{
	// void push_back(const ItemType& item)
	// Push one over capacity threshold.
	g_countCopy = g_countMove = 0;
	{
		AlignedVector< TestItem > items;

		// After 32 added items vector has to be rellocated,
		// thus should move 32 items.
		for (int i = 0; i < 33; ++i)
			items.push_back(TestItem(i));

		CASE_ASSERT(g_countCopy == 33);
		CASE_ASSERT(g_countMove == 32);
	}

	// iterator erase(const iterator& where)
	// Push one over capacity threshold.
	g_countCopy = g_countMove = 0;
	{
		AlignedVector< TestItem > items({ 1, 2, 3 });
		g_countCopy = g_countMove = 0;

		items.erase(items.begin());

		CASE_ASSERT(g_countCopy == 0);
		CASE_ASSERT(g_countMove == 2);
		CASE_ASSERT(items[0].value == 2);
		CASE_ASSERT(items[1].value == 3);
	}

	// iterator insert(const iterator& where, const ItemType& item)
	// Insert in the front.
	g_countCopy = g_countMove = 0;
	{
		AlignedVector< TestItem > items;

		for (int i = 0; i < 4; ++i)
			items.insert(items.begin(), TestItem(i));

		CASE_ASSERT(g_countCopy == 4);
		CASE_ASSERT(g_countMove == 6);
		CASE_ASSERT(items[0].value == 3);
		CASE_ASSERT(items[1].value == 2);
		CASE_ASSERT(items[2].value == 1);
		CASE_ASSERT(items[3].value == 0);
	}

	// iterator insert(const iterator& where, const IteratorType& from, const IteratorType& to)
	// Insert in the front.
	g_countCopy = g_countMove = 0;
	//{
	//	std::vector< TestItem > values({ 1, 2, 3 });
	//	g_countCopy = g_countMove = 0;

	//	AlignedVector< TestItem > items;
	//	items.push_back(4);
	//	items.insert(items.begin(), values.begin(), values.end());

	//	CASE_ASSERT(g_countCopy == 4);
	//	CASE_ASSERT(g_countMove == 1);
	//	CASE_ASSERT(items[0].value == 1);
	//	CASE_ASSERT(items[1].value == 2);
	//	CASE_ASSERT(items[2].value == 3);
	//	CASE_ASSERT(items[3].value == 4);
	//}

	// iterator insert(const iterator& where, const ItemType* from, const ItemType* to)
	// Insert in the front.
	g_countCopy = g_countMove = 0;
	{
		const TestItem values[] = { 1, 2, 3 };

		AlignedVector< TestItem > items;
		items.push_back(4);
		items.insert(items.begin(), &values[0], &values[3]);

		CASE_ASSERT(g_countCopy == 4);
		CASE_ASSERT(g_countMove == 1);
		CASE_ASSERT(items[0].value == 1);
		CASE_ASSERT(items[1].value == 2);
		CASE_ASSERT(items[2].value == 3);
		CASE_ASSERT(items[3].value == 4);
	}
}

}
