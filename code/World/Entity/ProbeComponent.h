#pragma once

#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

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

class Entity;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS ProbeComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	ProbeComponent(
		const resource::Proxy< render::ITexture >& probeDiffuseTexture,
		const resource::Proxy< render::ITexture >& probeSpecularTexture,
		const Aabb3& volume
	);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	Transform getTransform() const;

	void setProbeDiffuseTexture(const resource::Proxy< render::ITexture >& probeTexture) { m_probeDiffuseTexture = probeTexture; }

	const resource::Proxy< render::ITexture >& getProbeDiffuseTexture() const { return m_probeDiffuseTexture; }

	void setProbeSpecularTexture(const resource::Proxy< render::ITexture >& probeTexture) { m_probeSpecularTexture = probeTexture; }

	const resource::Proxy< render::ITexture >& getProbeSpecularTexture() const { return m_probeSpecularTexture; }

	const Aabb3& getVolume() const { return m_volume; }

private:
	Entity* m_owner;
	resource::Proxy< render::ITexture > m_probeDiffuseTexture;
	resource::Proxy< render::ITexture > m_probeSpecularTexture;
	Aabb3 m_volume;
};

	}
}
