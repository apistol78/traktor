#pragma once

#include "Core/Ref.h"
#include "World/IEntityComponentData.h"

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

class SplineComponent;

/*!
 * \ingroup Shape
 */
class T_DLLCLASS SplineComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< SplineComponent > createComponent() const;

	virtual void serialize(ISerializer& s) override final;
};

	}
}
