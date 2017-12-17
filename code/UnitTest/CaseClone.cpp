/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "UnitTest/CaseClone.h"

namespace traktor
{

class Clone_Base : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override
	{
		T_FATAL_ASSERT(s.getVersion< Clone_Base >() == 1);
		T_FATAL_ASSERT(s.getVersion() == 2);
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
		T_FATAL_ASSERT(s.getVersion() == 2);
	}
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"Clone_Base", 1, Clone_Base, ISerializable)

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"Clone_Derived", 2, Clone_Derived, Clone_Base)

void CaseClone::run()
{
	// Bianry serializer
	{
		Ref< Clone_Derived > src = new Clone_Derived();

		DynamicMemoryStream wms(false, true);
		BinarySerializer(&wms).writeObject(src);

		DynamicMemoryStream rms(wms.getBuffer(), true, false);
		Ref< Clone_Derived > dst = BinarySerializer(&rms).readObject< Clone_Derived >();
		T_FATAL_ASSERT (dst != 0);
	}

	// Deep clone
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
