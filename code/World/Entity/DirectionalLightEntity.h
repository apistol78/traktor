#pragma once

#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Directional light entity.
 * \ingroup World
 */
class T_DLLCLASS DirectionalLightEntity : public Entity
{
	T_RTTI_CLASS;

public:
	DirectionalLightEntity(
		const Transform& transform,
		const Vector4& color,
		bool castShadow
	);

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool getTransform(Transform& outTransform) const override final;

	virtual Aabb3 getBoundingBox() const override final;

	void setColor(const Vector4& color) { m_color = color; }

	const Vector4& getColor() const { return m_color; }

	void setCastShadow(bool castShadow) { m_castShadow = castShadow; }

	bool getCastShadow() const { return m_castShadow; }

private:
	Transform m_transform;
	Vector4 m_color;
	bool m_castShadow;
};

	}
}
