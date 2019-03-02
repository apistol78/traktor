#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionSuper.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionSuper", ActionSuper, ActionFunction)

ActionSuper::ActionSuper(ActionContext* context, ActionObject* object, ActionObject* superPrototype, ActionFunction* superClass)
:	ActionFunction(context, "super")
,	m_object(object)
,	m_superClass(superClass)
,	m_superPrototype(superPrototype)
{
	setOverrideRelay(m_object->getRelay());
}

ActionObject* ActionSuper::get__proto__()
{
	return m_superPrototype;
}

void ActionSuper::setMember(uint32_t memberName, const ActionValue& memberValue)
{
	T_FATAL_ERROR;
}

bool ActionSuper::getMember(uint32_t memberName, ActionValue& outMemberValue)
{
	if (!m_superPrototype)
		return false;

	Ref< ActionObject > prototype = m_object->get__proto__();
	m_object->set__proto__(m_superPrototype);
	bool result = m_object->getMember(memberName, outMemberValue);
	m_object->set__proto__(prototype);
	return result;
}

bool ActionSuper::getPropertyGet(uint32_t propertyName, Ref< ActionFunction >& outPropertyGet)
{
	if (!m_superPrototype)
		return false;

	Ref< ActionObject > prototype = m_object->get__proto__();
	m_object->set__proto__(m_superPrototype);
	bool result = m_object->getPropertyGet(propertyName, outPropertyGet);
	m_object->set__proto__(prototype);
	return result;
}

bool ActionSuper::getPropertySet(uint32_t propertyName, Ref< ActionFunction >& outPropertySet)
{
	if (!m_superPrototype)
		return false;

	Ref< ActionObject > prototype = m_object->get__proto__();
	m_object->set__proto__(m_superPrototype);
	bool result = m_object->getPropertySet(propertyName, outPropertySet);
	m_object->set__proto__(prototype);
	return result;
}

ActionValue ActionSuper::valueOf()
{
	return m_object->valueOf();
}

ActionValue ActionSuper::toString()
{
	return m_object->toString();
}

Ref< ActionObject > ActionSuper::getSuper()
{
	Ref< ActionFunction > superClass;
	ActionValue memberValue;

	// __proto__.__proto__
	Ref< ActionObject > superPrototype = m_superPrototype->get__proto__();
	if (m_superPrototype != 0 && superPrototype != m_superPrototype)
	{
		// __proto__.__ctor__
		if (m_superPrototype->getLocalMember(ActionContext::Id__ctor__, memberValue))
			superClass = memberValue.getObject< ActionFunction >();
		else
			superClass = dynamic_type_cast< ActionFunction* >(superPrototype);
	}
	else
		superPrototype = 0;

	return new ActionSuper(getContext(), m_object, superPrototype, superClass);
}

ActionValue ActionSuper::call(ActionObject* self, ActionObject* super, const ActionValueArray& args)
{
	T_ASSERT (self == m_object);

	if (!m_superPrototype || !m_superClass)
		return ActionValue();

	return m_superClass->call(self, getSuper(), args);
}

void ActionSuper::trace(visitor_t visitor) const
{
	visitor(m_superClass);
	visitor(m_superPrototype);
	visitor(m_object);
	ActionFunction::trace(visitor);
}

void ActionSuper::dereference()
{
	m_superClass = 0;
	m_superPrototype = 0;
	m_object = 0;
	ActionFunction::dereference();
}

	}
}
