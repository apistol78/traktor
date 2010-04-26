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
	const std::wstring& name,
	const uint8_t* code,
	uint16_t codeSize,
	ActionDictionary* dictionary
)
:	ActionFunction(name)
,	m_code(code)
,	m_codeSize(codeSize)
,	m_dictionary(dictionary)
{
	// Do this inside constructor to prevent infinite recursion.
	ActionValue classPrototype;
	if (AsFunction::getInstance()->getLocalMember(L"prototype", classPrototype))
		setMember(L"__proto__", classPrototype);
}

ActionValue ActionFunction1::call(const IActionVM* vm, ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
	ActionValuePool& pool = context->getPool();
	T_ANONYMOUS_VAR(ActionValuePool::Scope)(pool);

	ActionFrame callFrame(
		context,
		self,
		m_code,
		m_codeSize,
		4,
		m_dictionary,
		this
	);

	for (size_t i = 0; i < args.size(); ++i)
		callFrame.getStack().push(args[i]);

	vm->execute(&callFrame);

	ActionValueStack& callStack = callFrame.getStack();
	return !callStack.empty() ? callStack.top() : ActionValue();
}

ActionValue ActionFunction1::call(const IActionVM* vm, ActionFrame* callerFrame, ActionObject* self)
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
		m_code,
		m_codeSize,
		4,
		m_dictionary,
		this
	);

	//for (std::vector< std::wstring >::const_iterator i = m_arguments.begin(); i != m_arguments.end(); ++i)
	//	callFrame.setVariable(*i, callerStack.pop());

	for (int32_t i = 0; i < argCount; ++i)
		callFrame.getStack().push(args[i]);

	vm->execute(&callFrame);

	ActionValueStack& callStack = callFrame.getStack();
	return !callStack.empty() ? callStack.top() : ActionValue();
}

	}
}
