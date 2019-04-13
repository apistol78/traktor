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
		const resource::Proxy< render::ITexture >& diffuseTexture,
		const resource::Proxy< render::ITexture >& specularTexture,
		bool local,
		const Aabb3& volume
	);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	Transform getTransform() const;

	void setDiffuseTexture(const resource::Proxy< render::ITexture >& probeTexture) { m_diffuseTexture = probeTexture; }

	const resource::Proxy< render::ITexture >& getDiffuseTexture() const { return m_diffuseTexture; }

	void setSpecularTexture(const resource::Proxy< render::ITexture >& probeTexture) { m_specularTexture = probeTexture; }

	const resource::Proxy< render::ITexture >& getSpecularTexture() const { return m_specularTexture; }

	bool getLocal() const { return m_local; }

	const Aabb3& getVolume() const { return m_volume; }

private:
	Entity* m_owner;
	resource::Proxy< render::ITexture > m_diffuseTexture;
	resource::Proxy< render::ITexture > m_specularTexture;
	bool m_local;
	Aabb3 m_volume;
};

	}
}
