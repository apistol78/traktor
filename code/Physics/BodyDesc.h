/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_BodyDesc_H
#define traktor_physics_BodyDesc_H

#include <string>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class ShapeDesc;

/*! \brief Rigid body description.
 * \ingroup Physics
 */
class T_DLLCLASS BodyDesc : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setShape(ShapeDesc* shape);

	Ref< const ShapeDesc > getShape() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE;

private:
	Ref< ShapeDesc > m_shape;
};

	}
}

#endif	// traktor_physics_BodyDesc_H
