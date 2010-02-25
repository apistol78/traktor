#ifndef traktor_spray_PointSetSource_H
#define traktor_spray_PointSetSource_H

#include "Core/Math/Range.h"
#include "Resource/Proxy.h"
#include "Spray/PointSet.h"
#include "Spray/Source.h"

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

/*! \brief Point set particle source.
 * \ingroup Spray
 */
class T_DLLCLASS PointSetSource : public Source
{
	T_RTTI_CLASS;

public:
	PointSetSource();

	virtual bool bind(resource::IResourceManager* resourceManager);

	virtual void emit(
		EmitterUpdateContext& context,
		const Transform& transform,
		uint32_t emitCount,
		EmitterInstance& emitterInstance
	) const;

	virtual bool serialize(ISerializer& s);

	inline const resource::Proxy< PointSet >& getPointSet() const { return m_pointSet; }

	inline const Vector4& getOffset() const { return m_offset; }

private:
	mutable resource::Proxy< PointSet > m_pointSet;
	Vector4 m_offset;
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

	}
}

#endif	// traktor_spray_PointSetSource_H
