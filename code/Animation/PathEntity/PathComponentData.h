#pragma once

#include "Animation/PathEntity/PathComponent.h"
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

class PathComponent;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS PathComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< PathComponent > createComponent() const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	TransformPath m_path;
	PathComponent::TimeMode m_timeMode = PathComponent::TmManual;
	float m_timeOffset = 0.0f;
};

	}
}

