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
	const ActionDictionary* dictionary,
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
}

bool ActionFrame::hasVariable(uint32_t variableName) const
{
	SmallMap< uint32_t, ActionValue >::const_iterator i = m_localVariables.find(variableName);
	return i != m_localVariables.end();
}

void ActionFrame::setVariable(uint32_t variableName, const ActionValue& variableValue)
{
	m_localVariables[variableName] = variableValue;
}

bool ActionFrame::getVariable(uint32_t variableName, ActionValue& outVariableValue) const
{
	SmallMap< uint32_t, ActionValue >::const_iterator i = m_localVariables.find(variableName);
	if (i == m_localVariables.end())
		return false;

	outVariableValue = i->second;
	return true;
}

ActionValue* ActionFrame::getVariableValue(uint32_t variableName)
{
	SmallMap< uint32_t, ActionValue >::iterator i = m_localVariables.find(variableName);
	return (i != m_localVariables.end()) ? &i->second : 0;
}

bool ActionFrame::hasScopeVariable(uint32_t variableName) const
{
	SmallMap< uint32_t, ActionValue >::const_iterator i = m_scopeVariables.find(variableName);
	return i != m_localVariables.end();
}

void ActionFrame::setScopeVariables(const SmallMap< uint32_t, ActionValue >& variables)
{
	m_scopeVariables.insert(variables.begin(), variables.end());
}

void ActionFrame::setScopeVariable(uint32_t variableName, const ActionValue& variableValue)
{
	m_scopeVariables[variableName] = variableValue;
}

bool ActionFrame::getScopeVariable(uint32_t variableName, ActionValue& outVariableValue) const
{
	SmallMap< uint32_t, ActionValue >::const_iterator i = m_scopeVariables.find(variableName);
	if (i == m_scopeVariables.end())
		return false;

	outVariableValue = i->second;
	return true;
}

ActionValue* ActionFrame::getScopeVariableValue(uint32_t variableName)
{
	SmallMap< uint32_t, ActionValue >::iterator i = m_scopeVariables.find(variableName);
	return (i != m_scopeVariables.end()) ? &i->second : 0;
}

void ActionFrame::setDictionary(const ActionDictionary* dictionary)
{
	m_dictionary = dictionary;
}


	}
}