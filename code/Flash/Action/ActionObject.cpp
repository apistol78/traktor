#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Flash/Action/Avm1/ActionClass.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionObject.h"
#include "Flash/Action/ActionValueArray.h"
#include "Flash/Action/IActionObjectRelay.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionObject", ActionObject, Collectable)

ActionObject::ActionObject(ActionContext* context, IActionObjectRelay* relay)
:	m_context(context)
,	m_readOnly(false)
{
	setMember("__proto__", ActionValue("Object"));
	setRelay(relay);
}

ActionObject::ActionObject(ActionContext* context, const std::string& prototypeName, IActionObjectRelay* relay)
:	m_context(context)
,	m_readOnly(false)
{
	setMember("__proto__", ActionValue(prototypeName));
	setRelay(relay);
}

ActionObject::ActionObject(ActionContext* context, ActionObject* prototype, IActionObjectRelay* relay)
:	m_context(context)
,	m_readOnly(false)
{
	setMember("__proto__", ActionValue(prototype));
	setRelay(relay);
}

void ActionObject::addInterface(ActionObject* intrface)
{
	T_ASSERT (!m_readOnly);
}

void ActionObject::set__proto__(ActionObject* prototype)
{
	T_ASSERT (prototype != this);
	m__proto__ = prototype;
	m_members["__proto__"] = ActionValue(prototype);
}

ActionObject* ActionObject::get__proto__()
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
				m__proto__ = m_context->lookupClass(protoValue.getString());
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
			m__proto__ = m_context->lookupClass("Object");
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
				T_ASSERT (coerceValue.isObject< ActionClass >());
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
	else if (memberName == "prototype")
	{
		m_prototype = memberValue.getObject();
	}

	// Try setting through relay first.
	else if (m_relay)
	{
		if (m_relay->setMember(m_context, memberName, memberValue))
			return;
	}

	m_members[memberName] = memberValue;
}

bool ActionObject::getMember(const std::string& memberName, ActionValue& outMemberValue)
{
	if (getLocalMember(memberName, outMemberValue))
		return true;

	// Not a local member; try to get from our __proto__ reference.
	{
		Ref< ActionObject > __proto__ = get__proto__();
		if (__proto__)
		{
			T_ASSERT (__proto__ != this);

			while (__proto__)
			{
				if (__proto__->getLocalMember(memberName, outMemberValue))
					return true;

				Ref< ActionObject > parentPrototype = __proto__->get__proto__();
				__proto__ = (parentPrototype != __proto__) ? parentPrototype : 0;
			}
		}
	}

	// Not available through "__proto__" chain; assume class object and try from "prototype".
	{
		if (m_prototype)
		{
			if (m_prototype->getLocalMember(memberName, outMemberValue))
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

bool ActionObject::getPropertyGet(const std::string& propertyName, Ref< ActionFunction >& outPropertyGet)
{
	if (getLocalPropertyGet(propertyName, outPropertyGet))
		return true;

	// Not a local property; try to get from our __proto__ reference.
	{
		Ref< ActionObject > __proto__ = get__proto__();
		if (__proto__)
		{
			T_ASSERT (__proto__ != this);

			while (__proto__)
			{
				if (__proto__->getLocalPropertyGet(propertyName, outPropertyGet))
					return true;

				Ref< ActionObject > parentPrototype = __proto__->get__proto__();
				__proto__ = (parentPrototype != __proto__) ? parentPrototype : 0;
			}
		}
	}

	// Not available through "__proto__" chain; assume class object and try from "prototype".
	if (m_prototype)
	{
		if (m_prototype->getLocalPropertyGet(propertyName, outPropertyGet))
			return true;
	}

	return false;
}

bool ActionObject::getPropertySet(const std::string& propertyName, Ref< ActionFunction >& outPropertySet)
{
	T_ASSERT (!m_readOnly);

	if (getLocalPropertySet(propertyName, outPropertySet))
		return true;

	// Not a local property; try to get from our __proto__ reference.
	{
		Ref< ActionObject > __proto__ = get__proto__();
		if (__proto__)
		{
			T_ASSERT (__proto__ != this);
			while (__proto__)
			{
				if (__proto__->getLocalPropertySet(propertyName, outPropertySet))
					return true;

				Ref< ActionObject > parentPrototype = __proto__->get__proto__();
				__proto__ = (parentPrototype != __proto__) ? parentPrototype : 0;
			}
		}
	}

	// Not available through "__proto__" chain; assume class object and try from "prototype".
	if (m_prototype)
	{
		if (m_prototype->getLocalPropertySet(propertyName, outPropertySet))
			return true;
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

ActionValue ActionObject::valueOf()
{
	ActionValue valueOfFnV;
	if (getMember("valueOf", valueOfFnV))
	{
		ActionFunction* valueOfFn = valueOfFnV.getObject< ActionFunction >();
		if (valueOfFn)
		{
			ActionValueArray args;
			return valueOfFn->call(this, args);
		}
	}
	return ActionValue(avm_number_t(0));
}

ActionValue ActionObject::toString()
{
	ActionValue toStringFnV;
	if (getMember("toString", toStringFnV))
	{
		ActionFunction* toStringFn = toStringFnV.getObject< ActionFunction >();
		if (toStringFn)
		{
			ActionValueArray args;
			return toStringFn->call(this, args);
		}
	}
	return ActionValue("[object Object]");
}

void ActionObject::setReadOnly()
{
	m_readOnly = true;
}

bool ActionObject::hasOwnMember(const std::string& memberName) const
{
	member_map_t::const_iterator i = m_members.find(memberName);
	return i != m_members.end();
}

bool ActionObject::getLocalMember(const std::string& memberName, ActionValue& outMemberValue) const
{
	member_map_t::const_iterator i = m_members.find(memberName);
	if (i == m_members.end())
	{
		if (m_relay)
		{
			if (m_relay->getMember(m_context, memberName, outMemberValue))
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
			visitor(i->second.getObject());
	}

	for (property_map_t::const_iterator i = m_properties.begin(); i != m_properties.end(); ++i)
	{
		if (i->second.first)
			visitor(i->second.first);
		if (i->second.second)
			visitor(i->second.second);
	}

	visitor(m__proto__);
	visitor(m_prototype);
	visitor(m_relay);
}

void ActionObject::dereference()
{
	m_context = 0;
	m_members.clear();
	m_properties.clear();
	m__proto__ = 0;
	m_prototype = 0;
	m_relay = 0;
}

void ActionObject::setOverrideRelay(IActionObjectRelay* relay)
{
	m_relay = relay;
}

	}
}
