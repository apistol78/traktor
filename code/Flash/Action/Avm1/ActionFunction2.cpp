/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Flash/SpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionSuper.h"
#include "Flash/Action/IActionVMImage.h"
#include "Flash/Action/Avm1/ActionFunction2.h"
#include "Flash/Action/Common/Array.h"
#include "Flash/Action/Common/Classes/AsFunction.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunction2", ActionFunction2, ActionFunction)

ActionFunction2::ActionFunction2(
	ActionContext* context,
	const char* name,
	const IActionVMImage* image,
	uint8_t registerCount,
	uint16_t flags,
	const AlignedVector< std::pair< std::string, uint8_t > >& argumentsIntoRegisters,
	const SmallMap< uint32_t, ActionValue >& variables,
	const ActionDictionary* dictionary
)
:	ActionFunction(context, name)
,	m_image(image)
,	m_registerCount(registerCount)
,	m_flags(flags)
,	m_variables(variables)
,	m_dictionary(dictionary)
{
	for (AlignedVector< std::pair< std::string, uint8_t > >::const_iterator i = argumentsIntoRegisters.begin(); i != argumentsIntoRegisters.end(); ++i)
		m_argumentsIntoRegisters.push_back(std::make_pair(
			getContext()->getString(i->first),
			i->second
		));
}

ActionValue ActionFunction2::call(ActionObject* self, ActionObject* super, const ActionValueArray& args)
{
	T_ASSERT (!is_a< ActionSuper >(self));

	ActionContext* cx = getContext();
	ActionValuePool& pool = cx->getPool();
	T_ANONYMOUS_VAR(ActionValuePool::Scope)(pool);

	Ref< ActionObject > super2 = super;

	ActionFrame callFrame(
		cx,
		self,
		m_registerCount,
		m_dictionary,
		this
	);

	callFrame.setScopeVariables(m_variables);

	if (self)
	{
		if (!(m_flags & AffSuppressThis))
			callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		if (!(m_flags & AffSuppressSuper))
		{
			if (!super2)
				super2 = self->getSuper();
			callFrame.setVariable(ActionContext::IdSuper, ActionValue(super2));
		}
	}

	callFrame.setVariable(ActionContext::IdGlobal, ActionValue(cx->getGlobal()));

	// Preload registers.
	uint8_t preloadRegister = 1;
	if (m_flags & AffPreloadThis)
		callFrame.setRegister(preloadRegister++, ActionValue(self));

	if (m_flags & AffPreloadArguments || !(m_flags & AffSuppressArguments))
	{
		Ref< Array > argumentArray = new Array(args.size());
		for (uint32_t i = 0; i < args.size(); ++i)
			argumentArray->push(args[i]);
		if (m_flags & AffPreloadArguments)
			callFrame.setRegister(preloadRegister++, ActionValue(argumentArray->getAsObject(cx)));
		if (!(m_flags & AffSuppressArguments))
			callFrame.setVariable(ActionContext::IdArguments, ActionValue(argumentArray->getAsObject(cx)));
	}

	if (m_flags & AffPreloadSuper)
	{
		if (!super2 && self)
			super2 = self->getSuper();
		callFrame.setRegister(preloadRegister++, ActionValue(super2));
	}

	if (m_flags & AffPreloadRoot)
	{
		ActionValue root; 
		cx->getGlobal()->getLocalMember(ActionContext::IdRoot, root);
		callFrame.setRegister(preloadRegister++, root);
	}
	if (m_flags & AffPreloadParent)
	{
		if (self)
		{
			CharacterInstance* characterInstance = self->getRelay< CharacterInstance >();
			callFrame.setRegister(preloadRegister++, ActionValue(characterInstance->getParent()->getAsObject(cx)));
		}
		else
			callFrame.setRegister(preloadRegister++, ActionValue());
	}
	if (m_flags & AffPreloadGlobal)
		callFrame.setRegister(preloadRegister++, ActionValue(cx->getGlobal()));

	// Pass arguments into registers.
	uint32_t argumentPassed = 0;
	for (
		AlignedVector< std::pair< uint32_t, uint8_t > >::const_iterator i = m_argumentsIntoRegisters.begin();
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

	m_image->execute(&callFrame);

	return callStack.top();
}

void ActionFunction2::trace(visitor_t visitor) const
{
	for (SmallMap< uint32_t, ActionValue >::const_iterator i = m_variables.begin(); i != m_variables.end(); ++i)
	{
		if (i->second.isObjectStrong())
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
