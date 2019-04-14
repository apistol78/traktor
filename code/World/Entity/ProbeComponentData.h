#pragma once

#include "Core/Math/Aabb3.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"
//#include "World/WorldTypes.h"

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

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS ProbeComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	ProbeComponentData();

	virtual void serialize(ISerializer& s) override final;

	void setDiffuseTexture(const resource::Id< render::ITexture >& probeTexture) { m_diffuseTexture = probeTexture; }

	const resource::Id< render::ITexture >& getDiffuseTexture() const { return m_diffuseTexture; }

	void setSpecularTexture(const resource::Id< render::ITexture >& probeTexture) { m_specularTexture = probeTexture; }

	const resource::Id< render::ITexture >& getSpecularTexture() const { return m_specularTexture; }

	void setIntensity(float intensity) { m_intensity = intensity; }

	float getIntensity() const { return m_intensity; }

	void setLocal(bool local) { m_local = local; }

	bool getLocal() const { return m_local; }

	void setVolume(const Aabb3& volume) { m_volume = volume; }

	const Aabb3& getVolume() const { return m_volume; }

private:
	resource::Id< render::ITexture > m_diffuseTexture;
	resource::Id< render::ITexture > m_specularTexture;
	float m_intensity;
	bool m_local;
	Aabb3 m_volume;
};

	}
}
