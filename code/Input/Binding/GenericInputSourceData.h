#ifndef traktor_input_GenericInputSourceData_H
#define traktor_input_GenericInputSourceData_H

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
	
/*! \brief Generic input source.
 * \ingroup Input
 */
class T_DLLCLASS GenericInputSourceData : public IInputSourceData
{
	T_RTTI_CLASS;

public:
	GenericInputSourceData();
	
	GenericInputSourceData(
		InputCategory category,
		InputDefaultControlType controlType,
		bool analogue,
		bool normalize
	);

	GenericInputSourceData(
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

	void setNormalize(bool normalize);

	bool normalize() const;
	
	void setIndex(int32_t index);
	
	int32_t getIndex() const;
	
	virtual Ref< IInputSource > createInstance(DeviceControlManager* deviceControlManager) const;

	virtual void serialize(ISerializer& s);

private:
	InputCategory m_category;
	InputDefaultControlType m_controlType;
	bool m_analogue;
	bool m_normalize;
	int32_t m_index;
};

	}
}

#endif	// traktor_input_GenericInputSourceData_H
