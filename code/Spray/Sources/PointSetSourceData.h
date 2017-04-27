/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_PointSetSourceData_H
#define traktor_spray_PointSetSourceData_H

#include "Core/Math/Range.h"
#include "Core/Math/Vector4.h"
#include "Resource/Id.h"
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

class PointSet;

/*! \brief Point set particle source persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS PointSetSourceData : public SourceData
{
	T_RTTI_CLASS;

public:
	PointSetSourceData();

	virtual Ref< const Source > createSource(resource::IResourceManager* resourceManager) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< PointSet >& getPointSet() const { return m_pointSet; }

private:
	resource::Id< PointSet > m_pointSet;
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

#endif	// traktor_spray_PointSetSourceData_H
