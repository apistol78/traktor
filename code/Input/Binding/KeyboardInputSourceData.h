/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_KeyboardInputSourceData_H
#define traktor_input_KeyboardInputSourceData_H

#include <vector>
#include "Input/InputTypes.h"
#include "Input/Binding/IInputSourceData.h"

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
	
/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS KeyboardInputSourceData : public IInputSourceData
{
	T_RTTI_CLASS;

public:
	void addControlType(InputDefaultControlType controlType);

	const std::vector< InputDefaultControlType >& getControlTypes() const;
	
	virtual Ref< IInputSource > createInstance(DeviceControlManager* deviceControlManager) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::vector< InputDefaultControlType > m_controlTypes;
};

	}
}

#endif	// traktor_input_KeyboardInputSourceData_H
