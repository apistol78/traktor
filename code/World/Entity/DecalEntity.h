#ifndef traktor_world_DecalEntity_H
#define traktor_world_DecalEntity_H

#include "Core/Math/Float.h"
#include "Resource/Proxy.h"
#include "World/Entity/Entity.h"

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
		float alpha,
		const resource::Proxy< render::Shader >& shader
	);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	virtual void update(const UpdateParams& update);

	float getSize() const { return m_size; }

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	float getAlpha() const { return clamp(m_alpha * 2.0f, 0.0f, 1.0f); }

private:
	Transform m_transform;
	float m_size;
	float m_alpha;
	resource::Proxy< render::Shader > m_shader;
};

	}
}

#endif	// traktor_world_DecalEntity_H
