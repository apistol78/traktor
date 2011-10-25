#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/IActionVM.h"
#include "Flash/Action/Avm1/ActionFunction1.h"
#include "Flash/Action/Avm1/ActionSuper.h"
#include "Flash/Action/Avm1/Classes/AsFunction.h"
#include "Flash/Action/Classes/Array.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunction1", ActionFunction1, ActionFunction)

ActionFunction1::ActionFunction1(
	ActionContext* context,
	const std::string& name,
	const IActionVMImage* image,
	uint16_t argumentCount,
	const std::vector< std::string >& argumentsIntoVariables,
	ActionDictionary* dictionary
)
:	ActionFunction(context, name)
,	m_image(image)
,	m_argumentCount(argumentCount)
,	m_argumentsIntoVariables(argumentsIntoVariables)
,	m_dictionary(dictionary)
{
}

ActionValue ActionFunction1::call(ActionObject* self, const ActionValueArray& args)
{
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

	ActionValueStack& callStack = callFrame.getStack();

	Ref< Array > argumentArray = new Array();

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

	callFrame.setVariable("arguments", ActionValue(argumentArray->getAsObject(getContext())));
	callFrame.setVariable("super", ActionValue(new ActionSuper(getContext(), self)));

	getContext()->getVM()->execute(&callFrame);

	return callStack.top();
}

ActionValue ActionFunction1::call(ActionFrame* callerFrame, ActionObject* self)
{
	ActionValuePool& pool = getContext()->getPool();
	T_ANONYMOUS_VAR(ActionValuePool::Scope)(pool);

	ActionValueStack& callerStack = callerFrame->getStack();
	int32_t argCount = !callerStack.empty() ? int32_t(callerStack.pop().getNumber()) : 0;

	ActionValueArray args(getContext()->getPool(), argCount);
	for (int32_t i = 0; i < argCount; ++i)
		args[i] = callerStack.pop();

	ActionFrame callFrame(
		getContext(),
		self,
		m_image,
		4,
		m_dictionary,
		this
	);

	ActionValueStack& callStack = callFrame.getStack();

	Ref< Array > argumentArray = new Array();

	for (size_t i = 0; i < args.size(); ++i)
	{
		argumentArray->push(args[i]);
		callStack.push(args[i]);
	}

	for (size_t i = argCount; i < m_argumentCount; ++i)
		callStack.push(ActionValue());

	for (size_t i = 0; i < args.size(); ++i)
	{
		if (i >= m_argumentsIntoVariables.size())
			break;

		callFrame.setVariable(m_argumentsIntoVariables[i], args[i]);
	}

	callFrame.setVariable("arguments", ActionValue(argumentArray->getAsObject(getContext())));
	callFrame.setVariable("super", ActionValue(new ActionSuper(getContext(), self)));

	getContext()->getVM()->execute(&callFrame);

	return callStack.top();
}

	}
}
