#ifndef traktor_spray_EffectEntity_H
#define traktor_spray_EffectEntity_H

#include <set>
#include "Core/Math/Plane.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "Spray/Types.h"
#include "World/Entity.h"

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

class ISoundPlayer;

	}

	namespace spray
	{

class Effect;
class EffectInstance;
class MeshRenderer;
class PointRenderer;
class TrailRenderer;

/*! \brief Effect entity.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	EffectEntity(const Transform& transform, const resource::Proxy< Effect >& effect, sound::ISoundPlayer* soundPlayer);

	void render(
		const Vector4& cameraPosition,
		const Plane& cameraPlane,
		PointRenderer* pointRenderer,
		MeshRenderer* meshRenderer,
		TrailRenderer* trailRenderer
	);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	virtual Aabb3 getWorldBoundingBox() const;

	virtual void update(const world::UpdateParams& update);

	bool isFinished() const;

	const resource::Proxy< Effect >& getEffect() const { return m_effect; }

	void setEnable(bool enable) { m_enable = enable; }

	bool isEnable() const { return m_enable; }

	bool haveTechnique(render::handle_t technique) const { return m_techniques.find(technique) != m_techniques.end(); }

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
