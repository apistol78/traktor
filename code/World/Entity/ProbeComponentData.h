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

	void setProbeDiffuseTexture(const resource::Id< render::ITexture >& probeTexture) { m_probeDiffuseTexture = probeTexture; }

	const resource::Id< render::ITexture >& getProbeDiffuseTexture() const { return m_probeDiffuseTexture; }

	void setProbeSpecularTexture(const resource::Id< render::ITexture >& probeTexture) { m_probeSpecularTexture = probeTexture; }

	const resource::Id< render::ITexture >& getProbeSpecularTexture() const { return m_probeSpecularTexture; }

	void setVolume(const Aabb3& volume) { m_volume = volume; }

	const Aabb3& getVolume() const { return m_volume; }

private:
	resource::Id< render::ITexture > m_probeDiffuseTexture;
	resource::Id< render::ITexture > m_probeSpecularTexture;
	Aabb3 m_volume;
};

	}
}
