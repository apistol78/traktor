/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_IEntityComponentData_H
#define traktor_world_IEntityComponentData_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Component data interface.
 * \ingroup World
 */
class T_DLLCLASS IEntityComponentData : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_world_IEntityComponentData_H
