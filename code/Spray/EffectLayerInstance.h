#ifndef traktor_spray_EffectLayerInstance_H
#define traktor_spray_EffectLayerInstance_H

#include "Core/Object.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Aabb3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
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

	void update(Context& context, const Transform& transform, float time, bool enable);

	void synchronize();

	void render(
		PointRenderer* pointRenderer,
		MeshRenderer* meshRenderer,
		TrailRenderer* trailRenderer,
		const Transform& transform,
		const Vector4& cameraPosition,
		const Plane& cameraPlane,
		float time
	) const;

	Aabb3 getBoundingBox() const;

private:
	Ref< const EffectLayer > m_layer;
	Ref< EmitterInstance > m_emitterInstance;
	Ref< TrailInstance > m_trailInstance;
	Ref< SequenceInstance > m_sequenceInstance;
	float m_start;
	float m_end;
	bool m_singleShotFired;
	bool m_enable;
};

	}
}

#endif	// traktor_spray_EffectLayerInstance_H
