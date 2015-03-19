#ifndef traktor_spray_EffectLayerInstance_H
#define traktor_spray_EffectLayerInstance_H

#include "Core/Object.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Aabb3.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityEventInstance;

	}

	namespace spray
	{

struct Context;
class EffectLayer;
class EmitterInstance;
class MeshRenderer;
class PointRenderer;
class SequenceInstance;
class TrailInstance;
class TrailRenderer;

class T_DLLCLASS EffectLayerInstance : public Object
{
	T_RTTI_CLASS;

public:
	EffectLayerInstance(
		const EffectLayer* layer,
		EmitterInstance* emitterInstance,
		TrailInstance* trailInstance,
		SequenceInstance* sequenceInstance
	);

	virtual ~EffectLayerInstance();

	void update(Context& context, const Transform& transform, float time, bool enable);

	void synchronize();

	void render(
		render::handle_t technique,
		PointRenderer* pointRenderer,
		MeshRenderer* meshRenderer,
		TrailRenderer* trailRenderer,
		const Transform& transform,
		const Vector4& cameraPosition,
		const Plane& cameraPlane,
		float time
	) const;

	Aabb3 getBoundingBox() const;

	const EmitterInstance* getEmitterInstance() const { return m_emitterInstance; }

	const TrailInstance* getTrailInstance() const { return m_trailInstance; }

	const SequenceInstance* getSequenceInstance() const { return m_sequenceInstance; }

	const EffectLayer* getLayer() const { return m_layer; }

private:
	Ref< const EffectLayer > m_layer;
	Ref< EmitterInstance > m_emitterInstance;
	Ref< TrailInstance > m_trailInstance;
	Ref< SequenceInstance > m_sequenceInstance;
	Ref< world::IEntityEventInstance > m_triggerInstance;
	float m_start;
	float m_end;
	bool m_singleShotFired;
	bool m_enable;
};

	}
}

#endif	// traktor_spray_EffectLayerInstance_H
