#ifndef traktor_input_InputValueSourceData_H
#define traktor_input_InputValueSourceData_H

#include "Core/Serialization/ISerializable.h"
#include "Input/InputTypes.h"

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
	
class T_DLLCLASS InputValueSourceData : public ISerializable
{
	T_RTTI_CLASS;

public:
	InputValueSourceData();
	
	InputValueSourceData(
		InputCategory category,
		InputDefaultControlType controlType,
		const std::wstring& valueId
	);

	InputValueSourceData(
		InputCategory category,
		int32_t index,
		InputDefaultControlType controlType,
		const std::wstring& valueId
	);
	
	void setCategory(InputCategory category);
	
	InputCategory getCategory() const;
	
	void setControlType(InputDefaultControlType controlType);
	
	InputDefaultControlType getControlType() const;
	
	void setIndex(int32_t index);
	
	int32_t getIndex() const;
	
	void setValueId(const std::wstring& valueId);
	
	const std::wstring& getValueId() const;
	
	virtual bool serialize(ISerializer& s);

private:
	InputCategory m_category;
	InputDefaultControlType m_controlType;
	int32_t m_index;
	std::wstring m_valueId;
};

	}
}

#endif	// traktor_input_InputValueSourceData_H
