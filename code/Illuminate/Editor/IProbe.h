/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_illuminate_IProbe_H
#define traktor_illuminate_IProbe_H

#include "Core/Object.h"
#include "Core/Math/Color4f.h"

namespace traktor
{
	namespace illuminate
	{

class IProbe : public Object
{
	T_RTTI_CLASS;

public:
	virtual Color4f sample(const Vector4& direction) const = 0;
};

	}
}

#endif	// traktor_illuminate_IProbe_H
