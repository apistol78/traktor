#pragma once

#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace sb
	{

bool writeFileIfMismatch(const std::wstring& fileName, const AlignedVector< uint8_t >& data);

	}
}

