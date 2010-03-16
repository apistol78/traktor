#ifndef traktor_spray_EmitterInstance_H
#define traktor_spray_EmitterInstance_H

#include "Core/Object.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Aabb.h"
#include "Core/Thread/JobManager.h"
#include "Spray/EmitterUpdateContext.h"
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

#if defined(_PS3)
class SpursJobQueue;
#endif

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
	EmitterInstance(Emitter* emitter);

	virtual ~EmitterInstance();

	void update(EmitterUpdateContext& context, const Transform& transform, bool emit, bool singleShot);

	void render(PointRenderer* pointRenderer, const Plane& cameraPlane) const;

	void synchronize() const;

	inline void setTotalTime(float totalTime) { m_totalTime = totalTime; }

	inline float getTotalTime() const { return m_totalTime; }

	inline void addPoint(const Point& point) { m_points.push_back(point); m_emitted++; }

	inline const PointVector& getPoints() const { return m_points; }

	inline uint32_t getEmitted() const { return m_emitted; }

	inline const Aabb& getBoundingBox() const { return m_boundingBox; }

private:
	Ref< Emitter > m_emitter;
	float m_totalTime;
	PointVector m_points;
	uint32_t m_emitted;
	bool m_warm;
	Aabb m_boundingBox;
	mutable Job m_jobs[4];
	uint32_t m_count;

#if defined(_PS3)
	Ref< SpursJobQueue > m_jobQueue;
#else
	void updateTask(float deltaTime, const Transform& transform, size_t first, size_t last);
#endif
};

	}
}

#endif	// traktor_spray_EmitterInstance_H
