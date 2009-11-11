#ifndef traktor_flash_ActionFrame_H
#define traktor_flash_ActionFrame_H

#include <vector>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/ActionValueStack.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionContext;
class ActionObject;
class ActionDictionary;
class ActionFunction;

/*! \brief ActionScript execution stack frame.
 * \ingroup Flash
 */
class T_DLLCLASS ActionFrame : public Object
{
	T_RTTI_CLASS(ActionFrame)

public:
	ActionFrame(
		ActionContext* context,
		ActionObject* self,
		const uint8_t* code,
		uint16_t codeSize,
		uint16_t localRegisters,
		ActionDictionary* dictionary,
		ActionFunction* callee
	);

	void setRegister(uint16_t index, const ActionValue& value);

	ActionValue getRegister(uint16_t index) const;

	bool hasVariable(const std::wstring& variableName) const;

	void setVariable(const std::wstring& variableName, const ActionValue& variableValue);

	bool getVariable(const std::wstring& variableName, ActionValue& outVariableValue) const;

	void setDictionary(ActionDictionary* dictionary);

	inline Ref< ActionContext > getContext() const { return m_context; }

	inline Ref< ActionObject > getSelf() const { return m_self; }

	inline const uint8_t* getCode() const { return m_code; }

	inline uint16_t getCodeSize() const { return m_codeSize; }

	inline Ref< ActionDictionary > getDictionary() const { return m_dictionary; }

	inline Ref< ActionFunction > getCallee() const { return m_callee; }

	inline ActionValueStack& getStack() { return m_stack; }

private:
	Ref< ActionContext > m_context;
	Ref< ActionObject > m_self;
	const uint8_t* m_code;
	uint16_t m_codeSize;
	std::vector< ActionValue > m_localRegisters;
	std::map< std::wstring, ActionValue > m_localVariables;
	Ref< ActionDictionary > m_dictionary;
	Ref< ActionFunction > m_callee;
	ActionValueStack m_stack;
};

	}
}

#endif	// traktor_flash_ActionFrame_H
