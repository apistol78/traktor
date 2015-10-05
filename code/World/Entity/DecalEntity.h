#ifndef traktor_world_DecalEntity_H
#define traktor_world_DecalEntity_H

#include "Core/Math/Float.h"
#include "Resource/Proxy.h"
#include "World/Entity.h"

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace world
	{

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS DecalEntity : public Entity
{
	T_RTTI_CLASS;

public:
	DecalEntity(
		const Transform& transform,
		float size,
		float thickness,
		float alpha,
		float cullDistance,
		const resource::Proxy< render::Shader >& shader
	);

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	const Transform& getTransform() const { return m_transform; }

	float getSize() const { return m_size; }

	float getThickness() const { return m_thickness; }

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	float getAlpha() const { return clamp((m_alpha - m_age) * 2.0f, 0.0f, 1.0f); }

	float getCullDistance() const { return m_cullDistance; }

	float getAge() const { return m_age; }

private:
	Transform m_transform;
	float m_size;
	float m_thickness;
	float m_alpha;
	float m_cullDistance;
	float m_age;
	resource::Proxy< render::Shader > m_shader;
};

	}
}

#endif	// traktor_world_DecalEntity_H
