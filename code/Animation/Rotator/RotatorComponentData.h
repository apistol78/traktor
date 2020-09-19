#pragma once

#include "Animation/Rotator/RotatorComponent.h"
#include "Core/Ref.h"
#include "World/IEntityComponentData.h"

#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RotatorComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< RotatorComponent > createComponent() const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	RotatorComponent::Axis m_axis = RotatorComponent::Axis::X;
	float m_rate = 0.0f;
};

	}
}

