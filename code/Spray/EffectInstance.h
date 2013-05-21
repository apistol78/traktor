#ifndef traktor_spray_EffectInstance_H
#define traktor_spray_EffectInstance_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Transform.h"

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
class Effect;
class EffectLayerInstance;
class PointRenderer;
class TrailRenderer;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS EffectInstance : public Object
{
	T_RTTI_CLASS;

public:
	EffectInstance(const Effect* effect);

	void update(Context& context, const Transform& transform, bool enable);

	void synchronize();

	void render(
		PointRenderer* pointRenderer,
		TrailRenderer* trailRenderer,
		const Transform& transform,
		const Vector4& cameraPosition,
		const Plane& cameraPlane
	) const;

	void setTime(float time) { m_time = time; }

	float getTime() const { return m_time; }

	void setLoopEnable(bool loopEnable) { m_loopEnable = loopEnable; }

	bool getLoopEnable() const { return m_loopEnable; }

	const Aabb3& getBoundingBox() const { return m_boundingBox; }

private:
	friend class Effect;

	Ref< const Effect > m_effect;
	float m_time;
	bool m_loopEnable;
	Aabb3 m_boundingBox;
	RefArray< EffectLayerInstance > m_layerInstances;
};

	}
}

#endif	// traktor_spray_EffectInstance_H
