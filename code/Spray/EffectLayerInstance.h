#ifndef traktor_spray_EffectLayerInstance_H
#define traktor_spray_EffectLayerInstance_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Aabb.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class EffectLayer;
class EmitterInstance;
struct EmitterUpdateContext;
class PointRenderer;

class T_DLLCLASS EffectLayerInstance : public Object
{
	T_RTTI_CLASS(EffectLayerInstance)

public:
	EffectLayerInstance(const EffectLayer* layer, EmitterInstance* emitterInstance);

	void update(EmitterUpdateContext& context, const Matrix44& transform, float time, bool enable);

	void synchronize();

	void render(PointRenderer* pointRenderer, const Plane& cameraPlane, float time) const;

	const Aabb& getBoundingBox() const;

private:
	Ref< const EffectLayer > m_layer;
	Ref< EmitterInstance > m_emitterInstance;
	float m_start;
	float m_end;
};

	}
}

#endif	// traktor_spray_EffectLayerInstance_H
