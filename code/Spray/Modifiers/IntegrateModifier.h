#ifndef traktor_spray_IntegrateModifier_H
#define traktor_spray_IntegrateModifier_H

#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! \brief Integrate particle velocity modifier.
 * \ingroup Spray
 */
class IntegrateModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	IntegrateModifier(float timeScale, bool linear, bool angular);

#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const T_OVERRIDE T_FINAL;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const T_OVERRIDE T_FINAL;
#endif

private:
	Scalar m_timeScale;
	bool m_linear;
	bool m_angular;
};

	}
}

#endif	// traktor_spray_IntegrateModifier_H
