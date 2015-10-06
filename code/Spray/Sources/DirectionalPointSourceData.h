#ifndef traktor_spray_DirectionalPointSourceData_H
#define traktor_spray_DirectionalPointSourceData_H

#include "Core/Math/Range.h"
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

/*! \brief Directional point particle source persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS DirectionalPointSourceData : public SourceData
{
	T_RTTI_CLASS;

public:
	DirectionalPointSourceData();

	virtual Ref< const Source > createSource(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	Vector4 m_position;
	Vector4 m_direction;
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

	}
}

#endif	// traktor_spray_DirectionalPointSourceData_H
