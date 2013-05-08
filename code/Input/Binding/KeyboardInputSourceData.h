#ifndef traktor_input_KeyboardInputSourceData_H
#define traktor_input_KeyboardInputSourceData_H

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
	
	virtual Ref< IInputSource > createInstance(DeviceControlManager* deviceControlManager) const;

	virtual void serialize(ISerializer& s);

private:
	std::vector< InputDefaultControlType > m_controlTypes;
};

	}
}

#endif	// traktor_input_KeyboardInputSourceData_H
