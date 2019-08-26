#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Winding3.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{
	
class T_DLLCLASS IShape : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool createWindings(AlignedVector< Winding3 >& outWindings) const = 0;

	virtual void createAnchors(AlignedVector< Vector4 >& outAnchors) const = 0;
};

	}
}
