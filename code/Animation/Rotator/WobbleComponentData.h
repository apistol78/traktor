#pragma once

#include "Animation/Rotator/WobbleComponent.h"
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
class T_DLLCLASS WobbleComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< WobbleComponent > createComponent() const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_magnitude = 1.0f;
	float m_rate = 1.0f;
};

	}
}
