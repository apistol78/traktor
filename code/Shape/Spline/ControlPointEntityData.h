#pragma once

#include "Core/Ref.h"
#include "World/Entity/ComponentEntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityBuilder;

	}

	namespace shape
	{

class ControlPointEntity;

/*! \brief
 * \ingroup Shape
 */
class T_DLLCLASS ControlPointEntityData : public world::ComponentEntityData
{
	T_RTTI_CLASS;

public:
	Ref< ControlPointEntity > createEntity(const world::IEntityBuilder* builder) const;

	virtual void serialize(ISerializer& s) override final;
};

	}
}
