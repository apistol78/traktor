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

/*! Reflection probe component.
 * \ingroup World
 */
class T_DLLCLASS ProbeComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	ProbeComponent(
		const resource::Proxy< render::ITexture >& texture,
		float intensity,
		bool local,
		const Aabb3& volume
	);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	Transform getTransform() const;

	void setTexture(const resource::Proxy< render::ITexture >& texture) { m_texture = texture; }

	const resource::Proxy< render::ITexture >& getTexture() const { return m_texture; }

	float getIntensity() const { return m_intensity; }

	bool getLocal() const { return m_local; }

	const Aabb3& getVolume() const { return m_volume; }

private:
	Entity* m_owner;
	resource::Proxy< render::ITexture > m_texture;
	float m_intensity;
	bool m_local;
	Aabb3 m_volume;
};

	}
}
