#ifndef traktor_spray_Source_H
#define traktor_spray_Source_H

#include "Core/Serialization/Serializable.h"
#include "Core/Math/Matrix44.h"
#include "Spray/Point.h"

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
class EmitterInstance;

/*! \brief Particle source.
 * \ingroup Spray
 */
class T_DLLCLASS Source : public Serializable
{
	T_RTTI_CLASS(Source)

public:
	Source();

	virtual void emit(
		EmitterUpdateContext& context,
		const Matrix44& transform,
		uint32_t emitCount,
		EmitterInstance& emitterInstance
	) const = 0;

	virtual bool serialize(Serializer& s);

	inline float getRate() const { return m_rate; }

private:
	float m_rate;
};

	}
}

#endif	// traktor_spray_Source_H
