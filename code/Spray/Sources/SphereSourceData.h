#ifndef traktor_spray_SphereSourceData_H
#define traktor_spray_SphereSourceData_H

#include "Spray/SourceData.h"

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

/*! \brief Sphere particle source persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS SphereSourceData : public SourceData
{
	T_RTTI_CLASS;

public:
	SphereSourceData();

	virtual Ref< const Source > createSource(resource::IResourceManager* resourceManager) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Vector4 m_position;
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

#endif	// traktor_spray_SphereSourceData_H
