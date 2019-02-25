#pragma once

#include "World/Entity/ComponentEntity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{

/*! \brief
 * \ingroup Shape
 */
class T_DLLCLASS ControlPointEntity : public world::ComponentEntity
{
	T_RTTI_CLASS;

public:
	explicit ControlPointEntity(const Transform& transform);

	virtual void setTransform(const Transform& transform) override;

	virtual Aabb3 getBoundingBox() const override;

	bool checkDirty();

private:
	bool m_dirty;
};

	}
}

