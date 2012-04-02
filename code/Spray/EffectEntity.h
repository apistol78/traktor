#ifndef traktor_spray_EffectEntity_H
#define traktor_spray_EffectEntity_H

#include <set>
#include "Core/Math/Plane.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "Spray/Types.h"
#include "World/Entity/Entity.h"

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

	namespace sound
	{

class SurroundEnvironment;
class SoundSystem;

	}

	namespace spray
	{

class Effect;
class EffectInstance;
class PointRenderer;

/*! \brief Effect entity.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	EffectEntity(const Transform& transform, const resource::Proxy< Effect >& effect, sound::SoundSystem* soundSystem, sound::SurroundEnvironment* surroundEnvironment);

	void render(const Plane& cameraPlane, PointRenderer* pointRenderer);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	virtual Aabb3 getWorldBoundingBox() const;

	virtual void update(const UpdateParams& update);

	inline resource::Proxy< Effect >& getEffect() { return m_effect; }

	inline void setEnable(bool enable) { m_enable = enable; }

	inline bool isEnable() const { return m_enable; }

	inline bool haveTechnique(render::handle_t technique) const { return m_techniques.find(technique) != m_techniques.end(); }

private:
	Transform m_transform;
	resource::Proxy< Effect > m_effect;
	Ref< EffectInstance > m_effectInstance;
	std::set< render::handle_t > m_techniques;
	Context m_context;
	uint32_t m_counter;
	bool m_enable;
};

	}
}

#endif	// traktor_spray_EffectEntity_H
