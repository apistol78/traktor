#pragma once

#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class TransformPath;

	namespace shape
	{

class T_DLLCLASS SplineLayerComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	virtual void pathChanged(const TransformPath& path) = 0;
};

	}
}
