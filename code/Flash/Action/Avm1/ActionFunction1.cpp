#include "Core/Log/Log.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/IActionVM.h"
#include "Flash/Action/Avm1/ActionFunction1.h"
#include "Flash/Action/Avm1/ActionSuper.h"
#include "Flash/Action/Avm1/Classes/AsFunction.h"
#include "Flash/Action/Classes/Array.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunction1", ActionFunction1, ActionFunction)

ActionFunction1::ActionFunction1(
	ActionContext* context,
	const char* name,
	const IActionVMImage* image,
	uint16_t argumentCount,
	const std::vector< std::string >& argumentsIntoVariables,
	const SmallMap< uint32_t, ActionValue >& variables,
	const ActionDictionary* dictionary
)
:	ActionFunction(context, name)
,	m_image(image)
,	m_argumentCount(argumentCount)
,	m_variables(variables)
,	m_dictionary(dictionary)
{
	for (std::vector< std::string >::const_iterator i = argumentsIntoVariables.begin(); i != argumentsIntoVariables.end(); ++i)
		m_argumentsIntoVariables.push_back(
			getContext()->getString(*i)
		);
}

ActionValue ActionFunction1::call(ActionObject* self, ActionObject* super, const ActionValueArray& args)
{
	T_ASSERT (!is_a< ActionSuper >(self));

	ActionValuePool& pool = getContext()->getPool();
	T_ANONYMOUS_VAR(ActionValuePool::Scope)(pool);

	ActionFrame callFrame(
		getContext(),
		self,
		m_image,
		4,
		m_dictionary,
		this
	);

	callFrame.setScopeVariables(m_variables);

	ActionValueStack& callStack = callFrame.getStack();

	Ref< Array > argumentArray = new Array(args.size());
	for (size_t i = 0; i < args.size(); ++i)
	{
		argumentArray->push(args[i]);
		callStack.push(args[i]);
	}

	for (size_t i = args.size(); i < m_argumentCount; ++i)
		callStack.push(ActionValue());

	for (size_t i = 0; i < args.size(); ++i)
	{
		if (i >= m_argumentsIntoVariables.size())
			break;

		callFrame.setVariable(m_argumentsIntoVariables[i], args[i]);
	}

	if (self)
	{
		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		if (super)
			callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		else
			callFrame.setVariable(ActionContext::IdSuper, ActionValue(self->getSuper()));
	}

	callFrame.setVariable(ActionContext::IdGlobal, ActionValue(getContext()->getGlobal()));
	callFrame.setVariable(ActionContext::IdArguments, ActionValue(argumentArray->getAsObject(getContext())));

	getContext()->getVM()->execute(&callFrame);

	return callStack.top();
}

void ActionFunction1::trace(visitor_t visitor) const
{
	for (SmallMap< uint32_t, ActionValue >::const_iterator i = m_variables.begin(); i != m_variables.end(); ++i)
	{
		if (i->second.isObjectStrong())
			visitor(i->second.getObject());
	}
	ActionFunction::trace(visitor);
}

void ActionFunction1::dereference()
{
	m_variables.clear();
	ActionFunction::dereference();
}

	}
}
