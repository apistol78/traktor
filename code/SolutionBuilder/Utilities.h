/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef Utilities_H
#define Utilities_H

#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace sb
	{

bool writeFileIfMismatch(const std::wstring& fileName, const AlignedVector< uint8_t >& data);

	}
}

#endif	// Utilities_H
