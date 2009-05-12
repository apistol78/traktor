#ifndef traktor_spray_IntegrateModifier_H
#define traktor_spray_IntegrateModifier_H

#include "Spray/Modifier.h"

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

/*! \brief Integrate particle velocity modifier.
 * \ingroup Spray
 */
class T_DLLCLASS IntegrateModifier : public Modifier
{
	T_RTTI_CLASS(IntegrateModifier)

public:
	IntegrateModifier();

	virtual void update(const Scalar& deltaTime, const Matrix44& transform, PointVector& points, size_t first, size_t last) const;

	virtual bool serialize(Serializer& s);

private:
	Scalar m_timeScale;
};

	}
}

#endif	// traktor_spray_IntegrateModifier_H
