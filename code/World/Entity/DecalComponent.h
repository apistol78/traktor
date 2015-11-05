#ifndef traktor_world_DecalComponent_H
#define traktor_world_DecalComponent_H

#include "Core/Math/Float.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

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
class T_DLLCLASS DecalComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	DecalComponent(
		float size,
		float thickness,
		float alpha,
		float cullDistance,
		const resource::Proxy< render::Shader >& shader
	);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	float getSize() const { return m_size; }

	float getThickness() const { return m_thickness; }

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	float getAlpha() const { return clamp((m_alpha - m_age) * 2.0f, 0.0f, 1.0f); }

	float getCullDistance() const { return m_cullDistance; }

	float getAge() const { return m_age; }

	const Transform& getTransform() const { return m_transform; }

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

#endif	// traktor_world_DecalComponent_H
