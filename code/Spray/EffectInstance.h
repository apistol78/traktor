#ifndef traktor_spray_EffectInstance_H
#define traktor_spray_EffectInstance_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"
#include "Core/Heap/RefArray.h"
#include "Core/Math/Aabb.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Transform.h"

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

struct EmitterUpdateContext;
class PointRenderer;
class Effect;
class EffectLayerInstance;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS EffectInstance : public Object
{
	T_RTTI_CLASS(EffectInstance)

public:
	EffectInstance(const Effect* effect);

	void update(EmitterUpdateContext& context, const Transform& transform, bool enable);

	void synchronize();

	void render(PointRenderer* pointRenderer, const Plane& cameraPlane) const;

	inline void setTime(float time) { m_time = time; }

	inline float getTime() const { return m_time; }

	inline void setLoopEnable(bool loopEnable) { m_loopEnable = loopEnable; }

	inline bool getLoopEnable() const { return m_loopEnable; }

	inline const Aabb& getBoundingBox() const { return m_boundingBox; }

private:
	friend class Effect;

	Ref< const Effect > m_effect;
	float m_time;
	bool m_loopEnable;
	Aabb m_boundingBox;
	RefArray< EffectLayerInstance > m_layerInstances;
};

	}
}

#endif	// traktor_spray_EffectInstance_H
