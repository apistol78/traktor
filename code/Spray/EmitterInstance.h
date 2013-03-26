#ifndef traktor_spray_EmitterInstance_H
#define traktor_spray_EmitterInstance_H

#include "Core/Object.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Aabb3.h"
#include "Core/Thread/Job.h"
#include "Spray/Types.h"
#include "Spray/Modifier.h"
#include "Spray/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Job;

	namespace spray
	{

class Emitter;
class PointRenderer;

/*! \brief Emitter instance.
 * \ingroup Spray
 */
class T_DLLCLASS EmitterInstance : public Object
{
	T_RTTI_CLASS;

public:
	EmitterInstance(const Emitter* emitter, float duration);

	virtual ~EmitterInstance();

	void update(Context& context, const Transform& transform, bool emit, bool singleShot);

	void render(PointRenderer* pointRenderer, const Transform& transform, const Plane& cameraPlane);

	void synchronize() const;

	void setTotalTime(float totalTime) { m_totalTime = totalTime; }

	float getTotalTime() const { return m_totalTime; }

	void reservePoints(uint32_t npoints) { m_points.reserve(m_points.size() + npoints); }

	const PointVector& getPoints() const { return m_points; }

	//uint32_t getEmitted() const { return m_emitted; }

	const Aabb3& getBoundingBox() const { return m_boundingBox; }

	Point* addPoints(uint32_t points)
	{
		uint32_t offset = m_points.size();
		m_points.resize(offset + points);
		//m_emitted += points;
		return &m_points[offset];
	}

private:
	Ref< const Emitter > m_emitter;
	Vector4 m_position;
	PointVector m_points;
	PointVector m_renderPoints[4];
	//uint32_t m_emitted;
	float m_totalTime;
	float m_emitFraction;
	bool m_warm;
	Aabb3 m_boundingBox;
	uint32_t m_count;

#if !defined(T_MODIFIER_USE_PS3_SPURS)
	mutable Ref< Job > m_job;

	void updateTask(float deltaTime, const Transform& transform, size_t last);
#endif
};

	}
}

#endif	// traktor_spray_EmitterInstance_H
