#pragma once

#include "Core/Math/TransformPath.h"
#include "World/IEntityComponent.h"

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
class T_DLLCLASS PathComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	enum class TimeMode
	{
		Manual,
		Once,
		Loop,
		PingPong
	};

	explicit PathComponent(
		const TransformPath& path,
		TimeMode timeMode
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void continueTo(float time);

	const TransformPath& getPath() const { return m_path; }

	TimeMode getTimeMode() const { return m_timeMode; }

private:
	world::Entity* m_owner;
	Transform m_transform;
	TransformPath m_path;
	TimeMode m_timeMode;
	float m_time;
	float m_timeTarget;
};

	}
}

