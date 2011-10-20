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
	const IActionVMImage* image,
	uint16_t localRegisters,
	ActionDictionary* dictionary,
	ActionFunction* callee
)
:	m_context(context)
,	m_self(self)
,	m_image(image)
,	m_localRegisters(context->getPool(), localRegisters)
,	m_dictionary(dictionary)
,	m_callee(callee)
,	m_stack(context->getPool())
{
	setVariable("this", ActionValue(m_self));
	setVariable("_global", ActionValue(m_context->getGlobal()));
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

bool ActionFrame::hasVariable(const std::string& variableName) const
{
	std::map< std::string, ActionValue >::const_iterator i = m_localVariables.find(variableName);
	return i != m_localVariables.end();
}

void ActionFrame::setVariable(const std::string& variableName, const ActionValue& variableValue)
{
	m_localVariables[variableName] = variableValue;
}

bool ActionFrame::getVariable(const std::string& variableName, ActionValue& outVariableValue) const
{
	std::map< std::string, ActionValue >::const_iterator i = m_localVariables.find(variableName);
	if (i == m_localVariables.end())
		return false;

	outVariableValue = i->second;
	return true;
}

ActionValue* ActionFrame::getVariableValue(const std::string& variableName)
{
	std::map< std::string, ActionValue >::iterator i = m_localVariables.find(variableName);
	return (i != m_localVariables.end()) ? &i->second : 0;
}

void ActionFrame::setDictionary(ActionDictionary* dictionary)
{
	m_dictionary = dictionary;
}

	}
}