#ifndef traktor_spray_DiscSource_H
#define traktor_spray_DiscSource_H

#include "Spray/Source.h"
#include "Spray/Range.h"
#include "Core/Math/RandomGeometry.h"

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

/*! \brief Disc particle source.
 * \ingroup Spray
 */
class T_DLLCLASS DiscSource : public Source
{
	T_RTTI_CLASS;

public:
	DiscSource();

	virtual bool create(resource::IResourceManager* resourceManager);

	virtual void emit(
		EmitterUpdateContext& context,
		const Transform& transform,
		uint32_t emitCount,
		EmitterInstance& emitterInstance
	) const;

	virtual bool serialize(ISerializer& s);

	inline const Vector4& getPosition() const { return m_position; }

	inline const Vector4& getNormal() const { return m_normal; }

	inline const Range< float >& getRadius() const { return m_radius; }

private:
	Vector4 m_position;
	Vector4 m_normal;
	Range< float > m_radius;
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

	}
}

#endif	// traktor_spray_DiscSource_H
