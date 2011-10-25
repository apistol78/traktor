#include "Core/Log/Log.h"
#include "Flash/Action/Avm1/ActionSuper.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionSuper", ActionSuper, ActionFunction)

ActionSuper::ActionSuper(ActionContext* context, ActionObject* object)
:	ActionFunction(context, "super")
,	m_object(object)
{
	ActionValue memberValue;

	// __proto__
	Ref< ActionObject > prototype = m_object->get__proto__();

	// __proto__.__proto__
	m_superPrototype = prototype->get__proto__();
	if (m_superPrototype != prototype)
	{
		// __proto__.__ctor__
		if (prototype->getLocalMember("__ctor__", memberValue))
			m_superClass = memberValue.getObject< ActionFunction >();
		else
			m_superClass = dynamic_type_cast< ActionFunction* >(m_superPrototype);

		// Ensure relay instance is accessible through super object.
		setOverrideRelay(m_object->getRelay());
	}
	else
		m_superPrototype = 0;
}

ActionObject* ActionSuper::get__proto__()
{
	return m_superPrototype;
}

void ActionSuper::setMember(const std::string& memberName, const ActionValue& memberValue)
{
	m_object->setMember(memberName, memberValue);
}

bool ActionSuper::getMember(const std::string& memberName, ActionValue& outMemberValue)
{
	if (!m_superPrototype)
		return false;

	Ref< ActionObject > prototype = m_object->get__proto__();
	m_object->set__proto__(m_superPrototype);
	bool result = m_object->getMember(memberName, outMemberValue);
	m_object->set__proto__(prototype);
	return result;
}

bool ActionSuper::getPropertyGet(const std::string& propertyName, Ref< ActionFunction >& outPropertyGet)
{
	if (!m_superPrototype)
		return false;

	Ref< ActionObject > prototype = m_object->get__proto__();
	m_object->set__proto__(m_superPrototype);
	bool result = m_object->getPropertyGet(propertyName, outPropertyGet);
	m_object->set__proto__(prototype);
	return result;
}

bool ActionSuper::getPropertySet(const std::string& propertyName, Ref< ActionFunction >& outPropertySet)
{
	if (!m_superPrototype)
		return false;

	Ref< ActionObject > prototype = m_object->get__proto__();
	m_object->set__proto__(m_superPrototype);
	bool result = m_object->getPropertySet(propertyName, outPropertySet);
	m_object->set__proto__(prototype);
	return result;
}

ActionValue ActionSuper::call(ActionObject* self, const ActionValueArray& args)
{
	return ActionValue();
}

ActionValue ActionSuper::call(ActionFrame* callerFrame, ActionObject* self)
{
	if (!m_superPrototype || !m_superClass)
		return ActionValue();

	Ref< ActionObject > prototype = m_object->get__proto__();
	m_object->set__proto__(m_superPrototype);
	ActionValue ret = m_superClass->call(callerFrame, m_object);
	m_object->set__proto__(prototype);

	return ret;
}

void ActionSuper::trace(const IVisitor& visitor) const
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
