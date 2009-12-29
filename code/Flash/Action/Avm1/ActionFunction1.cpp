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
	const std::vector< std::wstring >& arguments,
	const uint8_t* code,
	uint16_t codeSize,
	ActionDictionary* dictionary
)
:	ActionFunction(name)
,	m_arguments(arguments)
,	m_code(code)
,	m_codeSize(codeSize)
,	m_dictionary(dictionary)
{
	// Do this inside constructor to prevent infinite recursion.
	ActionValue classPrototype;
	if (AsFunction::getInstance()->getLocalMember(L"prototype", classPrototype))
		setMember(L"__proto__", classPrototype);
}

ActionValue ActionFunction1::call(const IActionVM* vm, ActionFrame* callerFrame, ActionObject* self)
{
	ActionValueStack& callerStack = callerFrame->getStack();

	ActionFrame callFrame(
		callerFrame->getContext(),
		self,
		m_code,
		m_codeSize,
		4,
		m_dictionary,
		this
	);

	for (std::vector< std::wstring >::const_iterator i = m_arguments.begin(); i != m_arguments.end(); ++i)
		callFrame.setVariable(*i, callerStack.pop());

#if defined(_DEBUG)
	log::debug << L"ActionFunction1, enter VM" << Endl << IncreaseIndent;
#endif

	vm->execute(&callFrame);

#if defined(_DEBUG)
	log::debug << DecreaseIndent << L"ActionFunction1, returned from VM" << Endl;
#endif

	ActionValueStack& callStack = callFrame.getStack();
	return !callStack.empty() ? callStack.top() : ActionValue();
}

	}
}
