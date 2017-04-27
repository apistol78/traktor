/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Thread/Acquire.h"
#include "Flash/Action/ActionClass.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionObject.h"
#include "Flash/Action/ActionObjectRelay.h"
#include "Flash/Action/ActionSuper.h"
#include "Flash/Action/ActionValueArray.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionObject", ActionObject, Collectable)

ActionObject::ActionObject(ActionContext* context, ActionObjectRelay* relay)
:	m_context(context)
,	m_readOnly(false)
{
	setMember(ActionContext::Id__proto__, ActionValue("Object"));
	setRelay(relay);
}

ActionObject::ActionObject(ActionContext* context, const char* prototypeName, ActionObjectRelay* relay)
:	m_context(context)
,	m_readOnly(false)
{
	setMember(ActionContext::Id__proto__, ActionValue(prototypeName));
	setRelay(relay);
}

ActionObject::ActionObject(ActionContext* context, ActionObject* prototype, ActionObjectRelay* relay)
:	m_context(context)
,	m_readOnly(false)
{
	setMember(ActionContext::Id__proto__, ActionValue(prototype));
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
	m_members[ActionContext::Id__proto__] = ActionValue(prototype);
}

ActionObject* ActionObject::get__proto__()
{
	if (!m__proto__)
	{
		ActionValue protoValue;
		if (getLocalMember(ActionContext::Id__proto__, protoValue))
		{
			if (protoValue.isObject())
				m__proto__ = protoValue.getObject();
			else if (protoValue.isString())
			{
				m__proto__ = m_context->lookupClass(protoValue.getString());
				if (m__proto__)
				{
					// Replace string identifier with pointer to actual class.
					m_members[ActionContext::Id__proto__] = ActionValue(m__proto__);
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
				m_members[ActionContext::Id__proto__] = ActionValue(m__proto__);
			}
		}

		// Prepare object through special __init__ method if provided through prototype.
		if (m__proto__)
		{
			ActionValue initValue;
			if (m__proto__->getMember(ActionContext::IdConstructor, initValue))
			{
				ActionClass* builtinClass = initValue.getObject< ActionClass >();
				if (builtinClass)
					builtinClass->initialize(this);
			}
		}
	}

	return m__proto__;
}

void ActionObject::setMember(uint32_t memberName, const ActionValue& memberValue)
{
	// Reset cached pointer if __proto__ member is modified.
	if (memberName == ActionContext::Id__proto__)
		m__proto__ = 0;

	// Try setting through relay first.
	else if (m_relay)
	{
		if (m_relay->setMember(m_context, memberName, memberValue))
			return;
	}

	m_members[memberName] = memberValue;
}

bool ActionObject::getMember(uint32_t memberName, ActionValue& outMemberValue)
{
	if (getLocalMember(memberName, outMemberValue))
		return true;

	// Not a local member; try to get from our __proto__ reference.
	ActionObject* __proto__ = get__proto__();
	while (__proto__)
	{
		if (__proto__->getLocalMember(memberName, outMemberValue))
			return true;

		ActionObject* parentPrototype = __proto__->get__proto__();
		__proto__ = (parentPrototype != __proto__) ? parentPrototype : 0;
	}

	return false;
}

bool ActionObject::deleteMember(uint32_t memberName)
{
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

void ActionObject::addProperty(uint32_t propertyName, ActionFunction* propertyGet, ActionFunction* propertySet)
{
	m_properties[propertyName] = std::make_pair(propertyGet, propertySet);
}

bool ActionObject::getPropertyGet(uint32_t propertyName, Ref< ActionFunction >& outPropertyGet)
{
	if (getLocalPropertyGet(propertyName, outPropertyGet))
		return true;

	// Not a local property; try to get from our __proto__ reference.
	ActionObject* __proto__ = get__proto__();
	while (__proto__)
	{
		if (__proto__->getLocalPropertyGet(propertyName, outPropertyGet))
			return true;

		ActionObject* parentPrototype = __proto__->get__proto__();
		__proto__ = (parentPrototype != __proto__) ? parentPrototype : 0;
	}

	return false;
}

bool ActionObject::getPropertySet(uint32_t propertyName, Ref< ActionFunction >& outPropertySet)
{
	if (getLocalPropertySet(propertyName, outPropertySet))
		return true;

	// Not a local property; try to get from our __proto__ reference.
	ActionObject* __proto__ = get__proto__();
	while (__proto__)
	{
		if (__proto__->getLocalPropertySet(propertyName, outPropertySet))
			return true;

		ActionObject* parentPrototype = __proto__->get__proto__();
		__proto__ = (parentPrototype != __proto__) ? parentPrototype : 0;
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
	return ActionValue(0.0f);
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

Ref< ActionObject > ActionObject::getSuper()
{
	Ref< ActionFunction > superClass;
	ActionValue memberValue;

	// __proto__
	Ref< ActionObject > prototype = get__proto__();
	if (!prototype)
		return 0;

	// __proto__.__proto__
	Ref< ActionObject > superPrototype = prototype->get__proto__();
	if (superPrototype != prototype)
	{
		// __proto__.__ctor__
		if (prototype->getLocalMember(ActionContext::Id__ctor__, memberValue))
			superClass = memberValue.getObject< ActionFunction >();
		else
			superClass = dynamic_type_cast< ActionFunction* >(superPrototype);
	}
	else
		superPrototype = 0;

	return new ActionSuper(getContext(), this, superPrototype, superClass);
}

void ActionObject::setReadOnly()
{
	m_readOnly = true;
}

bool ActionObject::hasOwnMember(uint32_t memberName) const
{
	member_map_t::const_iterator i = m_members.find(memberName);
	return i != m_members.end();
}

bool ActionObject::getLocalMember(uint32_t memberName, ActionValue& outMemberValue) const
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

bool ActionObject::getMemberByQName(const std::string& memberName, ActionValue& outMemberValue)
{
	StringSplit< std::string > ss(memberName, ".");
	ActionValue objectValue(this);
	ActionValue memberValue;

	for (StringSplit< std::string >::const_iterator i = ss.begin(); i != ss.end(); ++i)
	{
		if (!objectValue.isObject() || !objectValue.getObject())
			return false;

		if (!objectValue.getObject()->getMember(*i, memberValue))
			return false;

		objectValue = memberValue;
	}

	outMemberValue = objectValue;
	return true;
}

bool ActionObject::hasOwnProperty(uint32_t propertyName) const
{
	property_map_t::const_iterator i = m_properties.find(propertyName);
	return i != m_properties.end();
}

bool ActionObject::getLocalPropertyGet(uint32_t propertyName, Ref< ActionFunction >& outPropertyGet) const
{
	property_map_t::const_iterator i = m_properties.find(propertyName);
	if (i == m_properties.end())
		return false;

	outPropertyGet = i->second.first;
	return true;
}

bool ActionObject::getLocalPropertySet(uint32_t propertyName, Ref< ActionFunction >& outPropertySet) const
{
	property_map_t::const_iterator i = m_properties.find(propertyName);
	if (i == m_properties.end())
		return false;

	outPropertySet = i->second.second;
	return true;
}

void ActionObject::setRelay(ActionObjectRelay* relay)
{
	if (m_relay)
		m_relay->setAsObject(0);
	if ((m_relay = relay) != 0)
		m_relay->setAsObject(this);
}

void ActionObject::setMember(const std::string& memberName, const ActionValue& memberValue)
{
	setMember(m_context->getString(memberName), memberValue);
}

bool ActionObject::getMember(const std::string& memberName, ActionValue& outMemberValue)
{
	return getMember(m_context->getString(memberName), outMemberValue);
}

bool ActionObject::getLocalMember(const std::string& memberName, ActionValue& outMemberValue) const
{
	return getLocalMember(m_context->getString(memberName), outMemberValue);
}

void ActionObject::addProperty(const std::string& propertyName, ActionFunction* propertyGet, ActionFunction* propertySet)
{
	addProperty(m_context->getString(propertyName), propertyGet, propertySet);
}

bool ActionObject::getPropertyGet(const std::string& propertyName, Ref< ActionFunction >& outPropertyGet)
{
	return getPropertyGet(m_context->getString(propertyName), outPropertyGet);
}

bool ActionObject::getPropertySet(const std::string& propertyName, Ref< ActionFunction >& outPropertySet)
{
	return getPropertySet(m_context->getString(propertyName), outPropertySet);
}

void ActionObject::trace(visitor_t visitor) const
{
	for (member_map_t::const_iterator i = m_members.begin(); i != m_members.end(); ++i)
	{
		if (i->second.isObjectStrong())
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
	visitor(m_relay);
}

void ActionObject::dereference()
{
	m_context = 0;
	m_members.clear();
	m_properties.clear();
	m__proto__ = 0;
	m_relay = 0;
}

void ActionObject::setOverrideRelay(ActionObjectRelay* relay)
{
	m_relay = relay;
}

	}
}
