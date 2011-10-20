#include "Core/Log/Log.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/IActionVM.h"
#include "Flash/Action/Avm1/ActionFunction2.h"
#include "Flash/Action/Avm1/ActionSuper.h"
#include "Flash/Action/Avm1/Classes/AsFunction.h"
#include "Flash/Action/Classes/Array.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunction2", ActionFunction2, ActionFunction)

ActionFunction2::ActionFunction2(
	ActionContext* context,
	const std::string& name,
	const IActionVMImage* image,
	uint8_t registerCount,
	uint16_t flags,
	const std::vector< std::pair< std::string, uint8_t > >& argumentsIntoRegisters,
	const std::map< std::string, ActionValue >& variables,
	ActionDictionary* dictionary
)
:	ActionFunction(context, name)
,	m_image(image)
,	m_registerCount(registerCount)
,	m_flags(flags)
,	m_argumentsIntoRegisters(argumentsIntoRegisters)
,	m_variables(variables)
,	m_dictionary(dictionary)
{
}

ActionValue ActionFunction2::call(ActionObject* self, const ActionValueArray& args)
{
	ActionContext* cx = getContext();
	ActionValuePool& pool = cx->getPool();
	T_ANONYMOUS_VAR(ActionValuePool::Scope)(pool);

	ActionFrame callFrame(
		cx,
		self,
		m_image,
		m_registerCount,
		m_dictionary,
		this
	);

	// Prepare activation scope variables; do this first
	// as some variables will get overridden below such as "this", "arguments" etc.
	for (std::map< std::string, ActionValue >::const_iterator i = m_variables.begin(); i != m_variables.end(); ++i)
		callFrame.setVariable(i->first, i->second);

	uint8_t preloadRegister = 1;
	if (m_flags & AffPreloadThis)
		callFrame.setRegister(preloadRegister++, ActionValue(self));
	//if (!(m_flags & AffSuppressThis))

	if (m_flags & AffPreloadArguments || !(m_flags & AffSuppressArguments))
	{
		Ref< Array > argumentArray = new Array();
		for (uint32_t i = 0; i < args.size(); ++i)
			argumentArray->push(args[i]);
		if (m_flags & AffPreloadArguments)
			callFrame.setRegister(preloadRegister++, ActionValue(argumentArray->getAsObject(cx)));
		if (!(m_flags & AffSuppressArguments))
			callFrame.setVariable("arguments", ActionValue(argumentArray->getAsObject(cx)));
	}

	if ((m_flags & AffPreloadSuper) || (!(m_flags & AffSuppressSuper)))
	{
		Ref< ActionSuper > super = new ActionSuper(cx, self);
		if (m_flags & AffPreloadSuper)
			callFrame.setRegister(preloadRegister++, ActionValue(super));
		if (!(m_flags & AffSuppressSuper))
			callFrame.setVariable("super", ActionValue(super));
	}

	if (m_flags & AffPreloadRoot)
	{
		ActionValue root; 
		cx->getGlobal()->getLocalMember("_root", root);
		callFrame.setRegister(preloadRegister++, root);
	}
	if (m_flags & AffPreloadParent)
	{
		FlashCharacterInstance* characterInstance = self->getRelay< FlashCharacterInstance >();
		callFrame.setRegister(preloadRegister++, ActionValue(characterInstance->getParent()->getAsObject(cx)));
	}
	if (m_flags & AffPreloadGlobal)
		callFrame.setRegister(preloadRegister++, ActionValue(cx->getGlobal()));

	// Pass arguments into registers.
	size_t argumentPassed = 0;
	for (
		std::vector< std::pair< std::string, uint8_t > >::const_iterator i = m_argumentsIntoRegisters.begin();
		argumentPassed < args.size() && i != m_argumentsIntoRegisters.end();
		++i
	)
	{
		if (i->second)
			callFrame.setRegister(i->second, args[argumentPassed++]);
		else
			callFrame.setVariable(i->first, args[argumentPassed++]);
	}

	ActionValueStack& callStack = callFrame.getStack();

	// Push rest of arguments onto stack.
	while (argumentPassed < args.size())
		callStack.push(args[argumentPassed++]);

	cx->getVM()->execute(&callFrame);

	return callStack.top();
}

ActionValue ActionFunction2::call(ActionFrame* callerFrame, ActionObject* self)
{
	ActionContext* cx = getContext();
	ActionValuePool& pool = cx->getPool();
	T_ANONYMOUS_VAR(ActionValuePool::Scope)(pool);

	ActionValueStack& callerStack = callerFrame->getStack();
	int32_t argCount = !callerStack.empty() ? int32_t(callerStack.pop().getNumber()) : 0;
	
	ActionValueArray args(cx->getPool(), argCount);
	for (int32_t i = 0; i < argCount; ++i)
		args[i] = callerStack.pop();

	ActionFrame callFrame(
		cx,
		self,
		m_image,
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
		for (uint32_t i = 0; i < args.size(); ++i)
			argumentArray->push(args[i]);
		if (m_flags & AffPreloadArguments)
			callFrame.setRegister(preloadRegister++, ActionValue(argumentArray->getAsObject(cx)));
		if (!(m_flags & AffSuppressArguments))
			callFrame.setVariable("arguments", ActionValue(argumentArray->getAsObject(cx)));
	}

	if ((m_flags & AffPreloadSuper) || (!(m_flags & AffSuppressSuper)))
	{
		Ref< ActionSuper > super = new ActionSuper(cx, self);
		if (m_flags & AffPreloadSuper)
			callFrame.setRegister(preloadRegister++, ActionValue(super));
		if (!(m_flags & AffSuppressSuper))
			callFrame.setVariable("super", ActionValue(super));
	}

	if (m_flags & AffPreloadRoot)
	{
		ActionValue root; 
		cx->getGlobal()->getLocalMember("_root", root);
		callFrame.setRegister(preloadRegister++, root);
	}
	if (m_flags & AffPreloadParent)
	{
		FlashCharacterInstance* characterInstance = self->getRelay< FlashCharacterInstance >();
		callFrame.setRegister(preloadRegister++, ActionValue(characterInstance->getParent()->getAsObject(cx)));
	}
	if (m_flags & AffPreloadGlobal)
		callFrame.setRegister(preloadRegister++, ActionValue(cx->getGlobal()));

	int32_t argumentPassed = 0;
	for (
		std::vector< std::pair< std::string, uint8_t > >::const_iterator i = m_argumentsIntoRegisters.begin();
		i != m_argumentsIntoRegisters.end();
		++i
	)
	{
		if (argumentPassed < argCount)
		{
			if (i->second)
				callFrame.setRegister(i->second, args[argumentPassed++]);
			else
				callFrame.setVariable(i->first, args[argumentPassed++]);
		}
		else
		{
			if (i->second)
				callFrame.setRegister(i->second, ActionValue());
			else
				callFrame.setVariable(i->first, ActionValue());

			++argumentPassed;
		}
	}

	ActionValueStack& callStack = callFrame.getStack();

	while (argumentPassed < argCount)
		callStack.push(args[argumentPassed++]);

	cx->getVM()->execute(&callFrame);

	return callStack.top();
}

void ActionFunction2::trace(const IVisitor& visitor) const
{
	for (std::map< std::string, ActionValue >::const_iterator i = m_variables.begin(); i != m_variables.end(); ++i)
	{
		if (i->second.isObject())
			visitor(i->second.getObject());
	}
	ActionFunction::trace(visitor);
}

void ActionFunction2::dereference()
{
	m_variables.clear();
	ActionFunction::dereference();
}

	}
}
