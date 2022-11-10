/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Test/CaseClone.h"

namespace traktor::test
{

class Clone_Base : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override
	{
		T_FATAL_ASSERT(s.getVersion< Clone_Base >() == 1);
	}
};

class Clone_Derived : public Clone_Base
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override
	{
		Clone_Base::serialize(s);
		T_FATAL_ASSERT(s.getVersion< Clone_Derived >() == 2);
	}
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseClone.Clone_Base", 1, Clone_Base, ISerializable)

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseClone.Clone_Derived", 2, Clone_Derived, Clone_Base)

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseClone", 0, CaseClone, Case)

void CaseClone::run()
{
	// Binary serializer.
	{
		Ref< Clone_Derived > src = new Clone_Derived();

		DynamicMemoryStream wms(false, true);
		bool written = false;
		{
			written = BinarySerializer(&wms).writeObject(src);
		}
		CASE_ASSERT (written);

		DynamicMemoryStream rms(wms.getBuffer(), true, false);
		Ref< Clone_Derived > dst = BinarySerializer(&rms).readObject< Clone_Derived >();

		CASE_ASSERT_NOT_EQUAL (dst, nullptr);
		if (dst)
		{
			CASE_ASSERT_EQUAL (dst->getReferenceCount(), 1);
		}
	}

	// Deep clone.
	{
		Ref< PropertyGroup > source = new PropertyGroup();
		Ref< PropertyBoolean > sourceChild = new PropertyBoolean();
		source->setProperty(L"Test", sourceChild);

		Ref< PropertyGroup > copy = DeepClone(source).create< PropertyGroup >();

		CASE_ASSERT_EQUAL(source->getReferenceCount(), 1);
		CASE_ASSERT_EQUAL(copy->getReferenceCount(), 1);
		CASE_ASSERT_EQUAL(sourceChild->getReferenceCount(), 2);
		CASE_ASSERT_NOT_EQUAL(copy->getProperty(L"Test"), sourceChild);
		CASE_ASSERT_EQUAL(copy->getProperty(L"Test")->getReferenceCount(), 1);
	}
}

}
