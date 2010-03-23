#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/IActionVM.h"
#include "Flash/Action/Avm1/ActionFunction2.h"
#include "Flash/Action/Avm1/ActionSuper.h"
#include "Flash/Action/Avm1/Classes/AsFunction.h"
#include "Flash/Action/Classes/Array.h"
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

ActionValue ActionFunction2::call(const IActionVM* vm, ActionContext* context, ActionObject* self, const std::vector< ActionValue >& args)
{
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
		callFrame.setRegister(preloadRegister++, ActionValue(self));
	//if (!(m_flags & AffSuppressThis))

	if (m_flags & AffPreloadArguments || !(m_flags & AffSuppressArguments))
	{
		Ref< Array > argumentArray = new Array();
		for (std::vector< ActionValue >::const_iterator i = args.begin(); i != args.end(); ++i)
			argumentArray->push(*i);
		if (m_flags & AffPreloadArguments)
			callFrame.setRegister(preloadRegister++, ActionValue(argumentArray));
		if (!(m_flags & AffSuppressArguments))
			callFrame.setVariable(L"arguments", ActionValue(argumentArray));
	}

	if ((m_flags & AffPreloadSuper) || (!(m_flags & AffSuppressSuper)))
	{
		Ref< ActionSuper > super = new ActionSuper(self);
		if (m_flags & AffPreloadSuper)
			callFrame.setRegister(preloadRegister++, ActionValue(super));
		if (!(m_flags & AffSuppressSuper))
			callFrame.setVariable(L"super", ActionValue(super));
	}

	if (m_flags & AffPreloadRoot)
	{
		ActionValue root; 
		context->getGlobal()->getMember(L"_root", root);
		callFrame.setRegister(preloadRegister++, root);
	}
	if (m_flags & AffPreloadParent)
	{
		FlashCharacterInstance* characterInstance = checked_type_cast< FlashCharacterInstance*, false >(self);
		callFrame.setRegister(preloadRegister++, ActionValue(characterInstance->getParent()));
	}
	if (m_flags & AffPreloadGlobal)
		callFrame.setRegister(preloadRegister++, ActionValue(context->getGlobal()));

	size_t argumentPassed = 0;
	for (
		std::vector< std::pair< std::wstring, uint8_t > >::const_iterator i = m_argumentsIntoRegisters.begin();
		argumentPassed < args.size() && i != m_argumentsIntoRegisters.end();
		++i
	)
	{
		if (i->second)
			callFrame.setRegister(i->second, args[argumentPassed++]);
		else
			callFrame.setVariable(i->first, args[argumentPassed++]);
	}
	while (argumentPassed < args.size())
		callFrame.getStack().push(args[argumentPassed++]);

	vm->execute(&callFrame);

	ActionValueStack& callStack = callFrame.getStack();
	return !callStack.empty() ? callStack.top() : ActionValue();
}

ActionValue ActionFunction2::call(const IActionVM* vm, ActionFrame* callerFrame, ActionObject* self)
{
	Ref< ActionContext > context = callerFrame->getContext();

	ActionValueStack& callerStack = callerFrame->getStack();
	int32_t argCount = !callerStack.empty() ? int32_t(callerStack.pop().getNumber()) : 0;
	
	args_t args(argCount);
	for (int32_t i = 0; i < argCount; ++i)
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
		callFrame.setRegister(preloadRegister++, ActionValue(self));
	//if (!(m_flags & AffSuppressThis))

	if (m_flags & AffPreloadArguments || !(m_flags & AffSuppressArguments))
	{
		Ref< Array > argumentArray = new Array();
		for (std::vector< ActionValue >::const_iterator i = args.begin(); i != args.end(); ++i)
			argumentArray->push(*i);
		if (m_flags & AffPreloadArguments)
			callFrame.setRegister(preloadRegister++, ActionValue(argumentArray));
		if (!(m_flags & AffSuppressArguments))
			callFrame.setVariable(L"arguments", ActionValue(argumentArray));
	}

	if ((m_flags & AffPreloadSuper) || (!(m_flags & AffSuppressSuper)))
	{
		Ref< ActionSuper > super = new ActionSuper(self);
		if (m_flags & AffPreloadSuper)
			callFrame.setRegister(preloadRegister++, ActionValue(super));
		if (!(m_flags & AffSuppressSuper))
			callFrame.setVariable(L"super", ActionValue(super));
	}

	if (m_flags & AffPreloadRoot)
	{
		ActionValue root; 
		context->getGlobal()->getMember(L"_root", root);
		callFrame.setRegister(preloadRegister++, root);
	}
	if (m_flags & AffPreloadParent)
	{
		FlashCharacterInstance* characterInstance = checked_type_cast< FlashCharacterInstance*, false >(self);
		callFrame.setRegister(preloadRegister++, ActionValue(characterInstance->getParent()));
	}
	if (m_flags & AffPreloadGlobal)
		callFrame.setRegister(preloadRegister++, ActionValue(context->getGlobal()));

	int32_t argumentPassed = 0;
	for (
		std::vector< std::pair< std::wstring, uint8_t > >::const_iterator i = m_argumentsIntoRegisters.begin();
		argumentPassed < argCount && i != m_argumentsIntoRegisters.end();
		++i
	)
	{
		if (i->second)
			callFrame.setRegister(i->second, args[argumentPassed++]);
		else
			callFrame.setVariable(i->first, args[argumentPassed++]);
	}
	while (argumentPassed < argCount)
		callFrame.getStack().push(args[argumentPassed++]);

	vm->execute(&callFrame);

	ActionValueStack& callStack = callFrame.getStack();
	return !callStack.empty() ? callStack.top() : ActionValue();
}

	}
}
