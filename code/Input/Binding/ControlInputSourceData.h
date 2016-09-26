#ifndef traktor_input_ControlInputSourceData_H
#define traktor_input_ControlInputSourceData_H

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
	
/*! \brief Control input source.
 *
 * A control input source allows the graph
 * to query attributes of an input device such
 * as if it's connected etc.
 *
 * \ingroup Input
 */
class T_DLLCLASS ControlInputSourceData : public IInputSourceData
{
	T_RTTI_CLASS;

public:
	ControlInputSourceData();
	
	ControlInputSourceData(
		InputCategory category,
		InputDefaultControlType controlType,
		bool analogue,
		bool normalize
	);

	ControlInputSourceData(
		InputCategory category,
		int32_t index,
		InputDefaultControlType controlType,
		bool analogue,
		bool normalize
	);
	
	void setCategory(InputCategory category);
	
	InputCategory getCategory() const;
	
	void setControlType(InputDefaultControlType controlType);
	
	InputDefaultControlType getControlType() const;

	void setAnalogue(bool analogue);

	bool isAnalogue() const;

	void setIndex(int32_t index);
	
	int32_t getIndex() const;
	
	virtual Ref< IInputSource > createInstance(DeviceControlManager* deviceControlManager) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	InputCategory m_category;
	InputDefaultControlType m_controlType;
	bool m_analogue;
	int32_t m_index;
};

	}
}

#endif	// traktor_input_ControlInputSourceData_H
