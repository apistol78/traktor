#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/IActionVM.h"
#include "Flash/Action/Avm1/ActionFunction1.h"
#include "Flash/Action/Avm1/Classes/AsFunction.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunction1", ActionFunction1, ActionFunction)

ActionFunction1::ActionFunction1(
	const std::string& name,
	const IActionVMImage* image,
	uint16_t argumentCount,
	ActionDictionary* dictionary
)
:	ActionFunction(name)
,	m_image(image)
,	m_argumentCount(argumentCount)
,	m_dictionary(dictionary)
{
}

ActionValue ActionFunction1::call(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
	ActionValuePool& pool = context->getPool();
	T_ANONYMOUS_VAR(ActionValuePool::Scope)(pool);

	ActionFrame callFrame(
		context,
		self,
		m_image,
		4,
		m_dictionary,
		this
	);

	ActionValueStack& callStack = callFrame.getStack();

	for (size_t i = 0; i < args.size(); ++i)
		callStack.push(args[i]);
	for (size_t i = args.size(); i < m_argumentCount; ++i)
		callStack.push(ActionValue());

	context->getVM()->execute(&callFrame);

	return callStack.top();
}

ActionValue ActionFunction1::call(ActionFrame* callerFrame, ActionObject* self)
{
	ActionContext* context = callerFrame->getContext();

	ActionValuePool& pool = context->getPool();
	T_ANONYMOUS_VAR(ActionValuePool::Scope)(pool);

	ActionValueStack& callerStack = callerFrame->getStack();
	int32_t argCount = !callerStack.empty() ? int32_t(callerStack.pop().getNumber()) : 0;

	ActionValueArray args(context->getPool(), argCount);
	for (int32_t i = 0; i < argCount; ++i)
		args[i] = callerStack.pop();

	ActionFrame callFrame(
		context,
		self,
		m_image,
		4,
		m_dictionary,
		this
	);

	//for (std::vector< std::wstring >::const_iterator i = m_arguments.begin(); i != m_arguments.end(); ++i)
	//	callFrame.setVariable(*i, callerStack.pop());

	ActionValueStack& callStack = callFrame.getStack();

	for (int32_t i = 0; i < argCount; ++i)
		callStack.push(args[i]);
	for (size_t i = argCount; i < m_argumentCount; ++i)
		callStack.push(ActionValue());

	context->getVM()->execute(&callFrame);

	return callStack.top();
}

	}
}
