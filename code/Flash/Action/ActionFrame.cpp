#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFrame", ActionFrame, Object)

ActionFrame::ActionFrame(
	ActionContext* context,
	ActionObject* self,
	const uint8_t* code,
	uint16_t codeSize,
	uint16_t localRegisters,
	ActionDictionary* dictionary,
	ActionFunction* callee
)
:	m_context(context)
,	m_self(self)
,	m_code(code)
,	m_codeSize(codeSize)
,	m_localRegisters(context->getPool(), localRegisters)
,	m_dictionary(dictionary)
,	m_callee(callee)
,	m_stack(context->getPool())
{
}

void ActionFrame::setRegister(uint16_t index, const ActionValue& value)
{
	m_localRegisters[index] = value;
}

ActionValue ActionFrame::getRegister(uint16_t index) const
{
	T_ASSERT (index < m_localRegisters.size());
	return m_localRegisters[index];
}

bool ActionFrame::hasVariable(const std::wstring& variableName) const
{
	if (variableName == L"this")
		return true;

	std::map< std::wstring, ActionValue >::const_iterator i = m_localVariables.find(variableName);
	return i != m_localVariables.end();
}

void ActionFrame::setVariable(const std::wstring& variableName, const ActionValue& variableValue)
{
	T_ASSERT (variableName != L"this");
	m_localVariables[variableName] = variableValue;
}

bool ActionFrame::getVariable(const std::wstring& variableName, ActionValue& outVariableValue) const
{
	if (variableName == L"this")
	{
		outVariableValue = ActionValue(m_self);
		return true;
	}

	std::map< std::wstring, ActionValue >::const_iterator i = m_localVariables.find(variableName);
	if (i == m_localVariables.end())
		return false;

	outVariableValue = i->second;
	return true;
}

void ActionFrame::setDictionary(ActionDictionary* dictionary)
{
	m_dictionary = dictionary;
}

	}
}