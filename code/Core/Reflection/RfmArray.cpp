/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Reflection/RfmArray.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmArray", RfmArray, RfmCompound)

RfmArray::RfmArray(const wchar_t* name)
:	RfmCompound(name)
{
}

}
