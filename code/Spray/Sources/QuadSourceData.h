#ifndef traktor_spray_QuadSourceData_H
#define traktor_spray_QuadSourceData_H

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

/*! \brief Quad particle source persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS QuadSourceData : public SourceData
{
	T_RTTI_CLASS;

public:
	QuadSourceData();

	virtual Ref< Source > createSource(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	Vector4 m_center;
	Vector4 m_axis1;
	Vector4 m_axis2;
	Vector4 m_normal;
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

	}
}

#endif	// traktor_spray_QuadSourceData_H
