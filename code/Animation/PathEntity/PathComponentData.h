#ifndef traktor_animation_PathComponentData_H
#define traktor_animation_PathComponentData_H

#include "Animation/PathEntity/PathComponent.h"
#include "World/IEntityComponentData.h"

#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;

	}

	namespace animation
	{

class PathComponent;

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS PathComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	PathComponentData();

	Ref< PathComponent > createComponent(world::Entity* owner) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	TransformPath m_path;
	PathComponent::TimeMode m_timeMode;
	float m_timeOffset;
};

	}
}

#endif	// traktor_animation_PathComponentData_H
