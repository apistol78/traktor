#pragma once

#include "Core/Object.h"
#include "Core/Math/Color4f.h"

namespace traktor
{
	namespace shape
	{
	
class IProbe : public Object
{
	T_RTTI_CLASS;

public:
	virtual Color4f sample(const Vector4& direction) const = 0;
};
	
	}
}
