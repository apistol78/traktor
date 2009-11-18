#include "Flash/Action/ActionFunction2.h"
#include "Flash/Action/ActionVM.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionSuper.h"
#include "Flash/Action/ActionArray.h"
#include "Flash/Action/Classes/AsFunction.h"
#include "Flash/FlashSpriteInstance.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunction2", ActionFunction2, ActionFunction)

ActionFunction2::ActionFunction2(
	const std::wstring& name,
	const uint8_t* code,
	uint16_t codeSize,
	uint8_t registerCount,
	uint16_t flags,
	const std::vector< std::pair< std::wstring, uint8_t > >& argumentsIntoRegisters,
	ActionDictionary* dictionary
)
:	ActionFunction(name)
,	m_code(code)
,	m_codeSize(codeSize)
,	m_registerCount(registerCount)
,	m_flags(flags)
,	m_argumentsIntoRegisters(argumentsIntoRegisters)
,	m_dictionary(dictionary)
{
	// Do this inside constructor to prevent infinite recursion.
	ActionValue classPrototype;
	if (AsFunction::getInstance()->getLocalMember(L"prototype", classPrototype))
		setMember(L"__proto__", classPrototype);
}

ActionValue ActionFunction2::call(ActionVM* vm, ActionFrame* callerFrame, ActionObject* self)
{
	Ref< ActionContext > context = callerFrame->getContext();

	ActionValueStack& callerStack = callerFrame->getStack();
	int argCount = !callerStack.empty() ? int(callerStack.pop().getNumber()) : 0;
	
	std::vector< ActionValue > args(argCount);
	for (int i = 0; i < argCount; ++i)
		args[i] = callerStack.pop();

	ActionFrame callFrame(
		context,
		self,
		m_code,
		m_codeSize,
		m_registerCount,
		m_dictionary,
		this
	);
	
	uint8_t preloadRegister = 1;
	if (m_flags & AffPreloadThis)
	{
#if defined(_DEBUG)
		log::debug << L"AffPreloadThis (" << int32_t(preloadRegister) << L")" << Endl;
#endif
		callFrame.setRegister(preloadRegister++, ActionValue::fromObject(self));
	}
	if (!(m_flags & AffSuppressThis))
	{
#if defined(_DEBUG)
		log::debug << L"!AffSuppressThis" << Endl;
#endif
	}

	if (m_flags & AffPreloadArguments || !(m_flags & AffSuppressArguments))
	{
		Ref< ActionArray > argumentArray = new ActionArray();
		for (std::vector< ActionValue >::iterator i = args.begin(); i != args.end(); ++i)
			argumentArray->push(*i);
		if (m_flags & AffPreloadArguments)
		{
#if defined(_DEBUG)
			log::debug << L"AffPreloadArguments (" << int32_t(preloadRegister) << L")" << Endl;
#endif
			callFrame.setRegister(preloadRegister++, ActionValue::fromObject(argumentArray));
		}
		if (!(m_flags & AffSuppressArguments))
		{
#if defined(_DEBUG)
			log::debug << L"!AffSuppressArguments" << Endl;
#endif
			callFrame.setVariable(L"arguments", ActionValue::fromObject(argumentArray));
		}
	}

	if ((m_flags & AffPreloadSuper) || (!(m_flags & AffSuppressSuper)))
	{
		Ref< ActionSuper > super = new ActionSuper(self);
		if (m_flags & AffPreloadSuper)
		{
#if defined(_DEBUG)
			log::debug << L"AffPreloadSuper (" << int32_t(preloadRegister) << L")" << Endl;
#endif
			callFrame.setRegister(preloadRegister++, ActionValue::fromObject(super));
		}
		if (!(m_flags & AffSuppressSuper))
		{
#if defined(_DEBUG)
			log::debug << L"!AffSuppressSuper" << Endl;
#endif
			callFrame.setVariable(L"super", ActionValue::fromObject(super));
		}
	}

	if (m_flags & AffPreloadRoot)
	{
#if defined(_DEBUG)
		log::debug << L"AffPreloadRoot (" << int32_t(preloadRegister) << L")" << Endl;
#endif
		ActionValue root; 
		context->getGlobal()->getMember(L"_root", root);
		callFrame.setRegister(preloadRegister++, root);
	}
	if (m_flags & AffPreloadParent)
	{
#if defined(_DEBUG)
		log::debug << L"AffPreloadParent (" << int32_t(preloadRegister) << L")" << Endl;
#endif
		FlashCharacterInstance* characterInstance = checked_type_cast< FlashCharacterInstance* >(self);
		T_ASSERT (characterInstance);
		callFrame.setRegister(preloadRegister++, ActionValue::fromObject(characterInstance->getParent()));
	}
	if (m_flags & AffPreloadGlobal)
	{
#if defined(_DEBUG)
		log::info << L"AffPreloadGlobal (" << int32_t(preloadRegister) << L")" << Endl;
#endif
		callFrame.setRegister(preloadRegister++, ActionValue::fromObject(context->getGlobal()));
	}

	int argumentPassed = 0;
	for (std::vector< std::pair< std::wstring, uint8_t > >::const_iterator i = m_argumentsIntoRegisters.begin(); argumentPassed < argCount && i != m_argumentsIntoRegisters.end(); ++i)
	{
		if (i->second)
		{
#if defined(_DEBUG)
			log::debug << L"Pass argument " << i->first << L" in register " << int32_t(i->second) << Endl;
#endif
			callFrame.setRegister(i->second, args[argumentPassed++]);
		}
		else
			callFrame.setVariable(i->first, args[argumentPassed++]);
	}
	while (argumentPassed < argCount)
		callFrame.getStack().push(args[argumentPassed++]);

#if defined(_DEBUG)
	log::debug << L"ActionFunction2, enter VM" << Endl << IncreaseIndent;
#endif

	vm->execute(&callFrame);

#if defined(_DEBUG)
	log::debug << DecreaseIndent << L"ActionFunction2, returned from VM" << Endl;
#endif

	ActionValueStack& callStack = callFrame.getStack();
	return !callStack.empty() ? callStack.top() : ActionValue();
}

	}
}
