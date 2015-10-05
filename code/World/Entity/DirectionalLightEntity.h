#ifndef traktor_world_DirectionalLightEntity_H
#define traktor_world_DirectionalLightEntity_H

#include "Resource/Proxy.h"
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
	namespace render
	{

class ITexture;

	}

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
		const Vector4& sunColor,
		const Vector4& baseColor,
		const Vector4& shadowColor,
		const resource::Proxy< render::ITexture >& cloudShadowTexture,
		bool castShadow
	);

	virtual void update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	void setSunColor(const Vector4& sunColor) { m_sunColor = sunColor; }

	const Vector4& getSunColor() const { return m_sunColor; }

	void setBaseColor(const Vector4& baseColor) { m_baseColor = baseColor; }

	const Vector4& getBaseColor() const { return m_baseColor; }

	void setShadowColor(const Vector4& shadowColor) { m_shadowColor = shadowColor; }

	const Vector4& getShadowColor() const { return m_shadowColor; }

	void setCloudShadowTexture(const resource::Proxy< render::ITexture >& cloudShadowTexture) { m_cloudShadowTexture = cloudShadowTexture; }

	const resource::Proxy< render::ITexture >& getCloudShadowTexture() const { return m_cloudShadowTexture; }

	void setCastShadow(bool castShadow) { m_castShadow = castShadow; }

	bool getCastShadow() const { return m_castShadow; }

private:
	Transform m_transform;
	Vector4 m_sunColor;
	Vector4 m_baseColor;
	Vector4 m_shadowColor;
	resource::Proxy< render::ITexture > m_cloudShadowTexture;
	bool m_castShadow;
};

	}
}

#endif	// traktor_world_DirectionalLightEntity_H
