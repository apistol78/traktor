/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "UnitTest/CaseClone.h"

namespace traktor
{

void CaseClone::run()
{
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
