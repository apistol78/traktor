/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "UnitTest/CaseAtomic.h"
#include "Core/Thread/Atomic.h"

namespace traktor
{

void CaseAtomic::run()
{
	{
		int32_t value = 1;
		int32_t retval = Atomic::increment(value);
		CASE_ASSERT_EQUAL(retval, 2);
		CASE_ASSERT_EQUAL(value, 2);
	}

	{
		int32_t value = 3;
		int32_t retval = Atomic::decrement(value);
		CASE_ASSERT_EQUAL(retval, 2);
		CASE_ASSERT_EQUAL(value, 2);
	}

	{
		uint32_t value = 4;
		uint32_t retval = Atomic::exchange(value, 5);
		CASE_ASSERT_EQUAL(retval, 4);
		CASE_ASSERT_EQUAL(value, 5);
	}

	{
		uint64_t value = 5;
		uint64_t retval = Atomic::exchange(value, 6);
		CASE_ASSERT_EQUAL(retval, 5);
		CASE_ASSERT_EQUAL(value, 6);
	}
}

}
