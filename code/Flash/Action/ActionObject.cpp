#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Flash/Action/Avm1/ActionClass.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionObject.h"
#include "Flash/Action/IActionObjectRelay.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionObject", ActionObject, Collectable)

ActionObject::ActionObject(IActionObjectRelay* relay)
:	m_readOnly(false)
{
	setMember("__proto__", ActionValue("Object"));
	setRelay(relay);
}

ActionObject::ActionObject(const std::string& prototypeName, IActionObjectRelay* relay)
:	m_readOnly(false)
{
	setMember("__proto__", ActionValue(prototypeName));
	setRelay(relay);
}

ActionObject::ActionObject(ActionObject* prototype, IActionObjectRelay* relay)
:	m_readOnly(false)
{
	setMember("__proto__", ActionValue(prototype));
	setRelay(relay);
}

void ActionObject::addInterface(ActionObject* intrface)
{
	T_ASSERT (!m_readOnly);
}

ActionObject* ActionObject::getPrototype(ActionContext* context)
{
	if (!m__proto__)
	{
		ActionValue protoValue;

		if (getLocalMember("__proto__", protoValue))
		{
			if (protoValue.isObject())
				m__proto__ = protoValue.getObject();
			else if (protoValue.isString())
			{
				m__proto__ = context->lookupClass(protoValue.getString());
				if (m__proto__)
				{
					// Replace string identifier with pointer to actual class.
					m_members["__proto__"] = ActionValue(m__proto__);
				}
			}
		}

		// No prototype defined; assume plain object.
		if (!m__proto__)
		{
			m__proto__ = context->lookupClass("Object");
			if (m__proto__)
			{
				// Replace string identifier with pointer to actual class.
				m_members["__proto__"] = ActionValue(m__proto__);
			}
		}

		// Create relayed object if builtin type which can be coerced.
		if (!m_relay)
		{
			ActionValue coerceValue;
			if (m__proto__->getLocalMember("__coerce__", coerceValue))
			{
				ActionClass* builtinClass = coerceValue.getObject< ActionClass >();
				if (builtinClass)
					builtinClass->coerce(this);
			}
		}
	}

	return m__proto__;
}

void ActionObject::setMember(const std::string& memberName, const ActionValue& memberValue)
{
	T_ASSERT (!m_readOnly);
	
	// Reset cached pointer if __proto__ member is modified.
	if (memberName == "__proto__")
	{
		m__proto__ = 0;
		m_relay = 0;
	}

	// Try setting through relay first.
	else if (m_relay)
	{
		if (m_relay->setMember(memberName, memberValue))
			return;
	}

	m_members[memberName] = memberValue;
}

bool ActionObject::getMember(ActionContext* context, const std::string& memberName, ActionValue& outMemberValue)
{
	if (getLocalMember(memberName, outMemberValue))
		return true;

	// Not a local member; try to get from our __proto__ reference.
	{
		Ref< ActionObject > prototype = getPrototype(context);
		if (prototype)
		{
			T_ASSERT (prototype != this);

			while (prototype)
			{
				if (prototype->getLocalMember(memberName, outMemberValue))
					return true;

				Ref< ActionObject > parentPrototype = prototype->getPrototype(context);
				prototype = (parentPrototype != prototype) ? parentPrototype : 0;
			}
		}
	}

	// Not available through __proto__ chain; assume class object and try from "prototype".
	{
		ActionValue prototypeValue;
		if (getLocalMember("prototype", prototypeValue))
		{
			Ref< ActionObject > prototype = prototypeValue.getObject();
			T_ASSERT (prototype != this);
			T_ASSERT (prototype);

			if (prototype->getLocalMember(memberName, outMemberValue))
				return true;
		}
	}

	return false;
}

bool ActionObject::deleteMember(const std::string& memberName)
{
	T_ASSERT (!m_readOnly);

	member_map_t::iterator i = m_members.find(memberName);
	if (i == m_members.end())
		return false;

	m_members.erase(i);
	return true;
}

void ActionObject::deleteAllMembers()
{
	m_members.clear();
}

void ActionObject::addProperty(const std::string& propertyName, ActionFunction* propertyGet, ActionFunction* propertySet)
{
	T_ASSERT (!m_readOnly);
	m_properties[propertyName] = std::make_pair(propertyGet, propertySet);
}

bool ActionObject::getPropertyGet(ActionContext* context, const std::string& propertyName, Ref< ActionFunction >& outPropertyGet)
{
	if (getLocalPropertyGet(propertyName, outPropertyGet))
		return true;

	// Not a local property; try to get from our __proto__ reference.
	{
		Ref< ActionObject > prototype = getPrototype(context);
		if (prototype)
		{
			T_ASSERT (prototype != this);

			while (prototype)
			{
				if (prototype->getLocalPropertyGet(propertyName, outPropertyGet))
					return true;

				Ref< ActionObject > parentPrototype = prototype->getPrototype(context);
				prototype = (parentPrototype != prototype) ? parentPrototype : 0;
			}
		}
	}

	// Not available through __proto__ chain; assume class object and try from "prototype".
	{
		ActionValue prototypeValue;
		if (getLocalMember("prototype", prototypeValue))
		{
			Ref< ActionObject > prototype = prototypeValue.getObject();
			T_ASSERT (prototype != this);
			T_ASSERT (prototype);

			if (prototype->getLocalPropertyGet(propertyName, outPropertyGet))
				return true;
		}
	}

	return false;
}

bool ActionObject::getPropertySet(ActionContext* context, const std::string& propertyName, Ref< ActionFunction >& outPropertySet)
{
	T_ASSERT (!m_readOnly);

	if (getLocalPropertySet(propertyName, outPropertySet))
		return true;

	// Not a local property; try to get from our __proto__ reference.
	{
		Ref< ActionObject > prototype = getPrototype(context);
		if (prototype)
		{
			T_ASSERT (prototype != this);
			while (prototype)
			{
				if (prototype->getLocalPropertySet(propertyName, outPropertySet))
					return true;

				Ref< ActionObject > parentPrototype = prototype->getPrototype(context);
				prototype = (parentPrototype != prototype) ? parentPrototype : 0;
			}
		}
	}

	// Not available through __proto__ chain; assume class object and try from "prototype".
	{
		ActionValue prototypeValue;
		if (getLocalMember("prototype", prototypeValue))
		{
			Ref< ActionObject > prototype = prototypeValue.getObject();
			T_ASSERT (prototype != this);
			T_ASSERT (prototype);

			if (prototype->getLocalPropertySet(propertyName, outPropertySet))
				return true;
		}
	}

	return false;
}

const ActionObject::property_map_t& ActionObject::getProperties() const
{
	return m_properties;
}

void ActionObject::deleteAllProperties()
{
	m_properties.clear();
}

avm_number_t ActionObject::valueOf() const
{
	return 0.0;
}

ActionValue ActionObject::toString() const
{
	if (m_relay)
		return m_relay->toString();
	else
		return ActionValue("[object Object]");
}

void ActionObject::setReadOnly()
{
	m_readOnly = true;
}

bool ActionObject::getLocalMember(const std::string& memberName, ActionValue& outMemberValue) const
{
	member_map_t::const_iterator i = m_members.find(memberName);
	if (i == m_members.end())
	{
		if (m_relay)
		{
			if (m_relay->getMember(memberName, outMemberValue))
				return true;
		}
		return false;
	}

	outMemberValue = i->second;
	return true;
}

bool ActionObject::hasOwnProperty(const std::string& propertyName) const
{
	property_map_t::const_iterator i = m_properties.find(propertyName);
	return i != m_properties.end();
}

bool ActionObject::getLocalPropertyGet(const std::string& propertyName, Ref< ActionFunction >& outPropertyGet) const
{
	property_map_t::const_iterator i = m_properties.find(propertyName);
	if (i == m_properties.end())
		return false;

	outPropertyGet = i->second.first;
	return true;
}

bool ActionObject::getLocalPropertySet(const std::string& propertyName, Ref< ActionFunction >& outPropertySet) const
{
	property_map_t::const_iterator i = m_properties.find(propertyName);
	if (i == m_properties.end())
		return false;

	outPropertySet = i->second.second;
	return true;
}

void ActionObject::setRelay(IActionObjectRelay* relay)
{
	if (m_relay)
		m_relay->setAsObject(0);
	if ((m_relay = relay) != 0)
		m_relay->setAsObject(this);
}

void ActionObject::trace(const IVisitor& visitor) const
{
	for (member_map_t::const_iterator i = m_members.begin(); i != m_members.end(); ++i)
	{
		if (i->second.isObject())
			visitor(i->second.getObjectUnsafe());
	}

	for (property_map_t::const_iterator i = m_properties.begin(); i != m_properties.end(); ++i)
	{
		if (i->second.first)
			visitor(i->second.first);
		if (i->second.second)
			visitor(i->second.second);
	}

	visitor(m__proto__);
	visitor(m_relay);
}

void ActionObject::dereference()
{
	m_members.clear();
	m_properties.clear();
	m__proto__ = 0;
	m_relay = 0;
}

	}
}
