#ifndef traktor_spray_PointSourceData_H
#define traktor_spray_PointSourceData_H

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

/*! \brief Point particle source persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS PointSourceData : public SourceData
{
	T_RTTI_CLASS;

public:
	PointSourceData();

	virtual Ref< const Source > createSource(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	Vector4 m_position;
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

	}
}

#endif	// traktor_spray_PointSourceData_H
