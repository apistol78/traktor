#ifndef traktor_spray_EffectEntity_H
#define traktor_spray_EffectEntity_H

#include "Core/Math/Plane.h"
#include "World/Entity/SpatialEntity.h"
#include "Resource/Proxy.h"
#include "Spray/EmitterUpdateContext.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace spray
	{

class Effect;
class EffectInstance;
class PointRenderer;

/*! \brief Effect entity.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntity : public world::SpatialEntity
{
	T_RTTI_CLASS;

public:
	EffectEntity(resource::IResourceManager* resourceManager, const Transform& transform, const resource::Proxy< Effect >& effect);

	void render(const Plane& cameraPlane, PointRenderer* pointRenderer);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb getBoundingBox() const;

	virtual Aabb getWorldBoundingBox() const;

	virtual void update(const world::EntityUpdate* update);

	inline resource::Proxy< Effect >& getEffect() { return m_effect; }

	inline void setEnable(bool enable) { m_enable = enable; }

	inline bool isEnable() const { return m_enable; }

private:
	resource::IResourceManager* m_resourceManager;
	Transform m_transform;
	resource::Proxy< Effect > m_effect;
	Ref< EffectInstance > m_effectInstance;
	EmitterUpdateContext m_context;
	uint32_t m_counter;
	bool m_enable;
};

	}
}

#endif	// traktor_spray_EffectEntity_H
