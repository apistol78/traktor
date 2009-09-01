#ifndef traktor_spray_SplineSource_H
#define traktor_spray_SplineSource_H

#include "Spray/Source.h"
#include "Spray/Range.h"
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"

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

/*! \brief Spline particle source.
 * \ingroup Spray
 */
class T_DLLCLASS SplineSource : public Source
{
	T_RTTI_CLASS(SplineSource)

public:
	enum Mode
	{
		MdRandom,
		MdSequential
	};

	struct Key
	{
		float T;
		Vector4 value;

		bool serialize(Serializer& s);
	};

	SplineSource();

	virtual void emit(
		EmitterUpdateContext& context,
		const Transform& transform,
		uint32_t emitCount,
		EmitterInstance& emitterInstance
	) const;

	virtual bool serialize(Serializer& s);

	inline const AlignedVector< Key >& getKeys() const { return m_keys; }

private:
	AlignedVector< Key > m_keys;
	Mode m_mode;
	float m_sequentialSpeed;
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

	}
}

#endif	// traktor_spray_SplineSource_H
