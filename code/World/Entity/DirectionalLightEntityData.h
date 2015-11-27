#ifndef traktor_world_DirectionalLightEntityData_H
#define traktor_world_DirectionalLightEntityData_H

#include "Resource/Id.h"
#include "World/EntityData.h"

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

/*! \brief Directional light entity data.
 * \ingroup World
 */
class T_DLLCLASS DirectionalLightEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	DirectionalLightEntityData();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const Vector4& getSunColor() const { return m_sunColor; }

	const Vector4& getBaseColor() const { return m_baseColor; }

	const Vector4& getShadowColor() const { return m_shadowColor; }

	const resource::Id< render::ITexture >& getCloudShadowTexture() const { return m_cloudShadowTexture; }

	bool getCastShadow() const { return m_castShadow; }

private:
	Vector4 m_sunColor;
	Vector4 m_baseColor;
	Vector4 m_shadowColor;
	resource::Id< render::ITexture > m_cloudShadowTexture;
	bool m_castShadow;
};

	}
}

#endif	// traktor_world_DirectionalLightEntityData_H
