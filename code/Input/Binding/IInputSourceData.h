/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_IInputSourceData_H
#define traktor_input_IInputSourceData_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class DeviceControlManager;
class IInputSource;

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS IInputSourceData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IInputSource > createInstance(DeviceControlManager* deviceControlManager) const = 0;
};

	}
}

#endif	// traktor_input_IInputSourceData_H
