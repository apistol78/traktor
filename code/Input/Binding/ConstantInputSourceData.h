#ifndef traktor_input_ConstantInputSourceData_H
#define traktor_input_ConstantInputSourceData_H

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
	
/*! \brief Constant value input.
 * \ingroup Input
 */
class T_DLLCLASS ConstantInputSourceData : public IInputSourceData
{
	T_RTTI_CLASS;

public:
	ConstantInputSourceData();

	ConstantInputSourceData(float value);
	
	virtual Ref< IInputSource > createInstance(DeviceControlManager* deviceControlManager) const;

	virtual void serialize(ISerializer& s);

private:
	float m_value;
};

	}
}

#endif	// traktor_input_ConstantInputSourceData_H
