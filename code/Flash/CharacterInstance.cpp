/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/CharacterInstance.h"
#include "Flash/SwfTypes.h"
#include "Flash/Types.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.CharacterInstance", CharacterInstance, ActionObjectRelay)

int32_t CharacterInstance::ms_instanceCount = 0;

CharacterInstance::CharacterInstance(
	ActionContext* context,
	const char* const prototype,
	Dictionary* dictionary,
	CharacterInstance* parent
)
:	ActionObjectRelay(prototype)
,	m_context(context)
,	m_dictionary(dictionary)
,	m_parent(parent)
,	m_filterColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_filter(0)
,	m_blendMode(0)
,	m_visible(true)
,	m_enabled(true)
{
	Atomic::increment(ms_instanceCount);

	m_cxform.mul = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	m_cxform.add = Color4f(0.0f, 0.0f, 0.0f, 0.0f);

	m_transform = Matrix33::identity();
}

CharacterInstance::~CharacterInstance()
{
	if (m_context)
	{
		if (m_context->getFocus() == this)
			m_context->setFocus(0);
	}

	m_context = 0;
	m_parent = 0;
	m_eventScripts.clear();

	ActionObjectRelay::dereference();
	Atomic::decrement(ms_instanceCount);
}

int32_t CharacterInstance::getInstanceCount()
{
	return ms_instanceCount;
}

void CharacterInstance::destroy()
{
	if (m_context)
	{
		if (m_context->getFocus() == this)
			m_context->setFocus(0);
		if (m_context->getPressed() == this)
			m_context->setPressed(0);
		if (m_context->getRolledOver() == this)
			m_context->setRolledOver(0);
	}

	m_context = 0;
	m_parent = 0;
	m_eventScripts.clear();

	ActionObjectRelay::dereference();
}

void CharacterInstance::setParent(CharacterInstance* parent)
{
	m_parent = parent;
}

void CharacterInstance::setName(const std::string& name)
{
	m_name = name;
}

const std::string& CharacterInstance::getName() const
{
	return m_name;
}

void CharacterInstance::setCacheObject(IRefCount* cacheObject)
{
	m_cacheObject = cacheObject;
}

void CharacterInstance::setUserObject(IRefCount* userObject)
{
	m_userObject = userObject;
}

void CharacterInstance::clearCacheObject()
{
	m_cacheObject = 0;
}

std::string CharacterInstance::getTarget() const
{
	return m_parent ? (m_parent->getTarget() + "/" + getName()) : "";
}

void CharacterInstance::setColorTransform(const ColorTransform& cxform)
{
	clearCacheObject();
	m_cxform = cxform;
}

ColorTransform CharacterInstance::getFullColorTransform() const
{
	if (m_parent)
		return m_parent->getFullColorTransform() * m_cxform;
	else
		return m_cxform;
}

void CharacterInstance::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

Matrix33 CharacterInstance::getFullTransform() const
{
	if (m_parent)
		return m_parent->getFullTransform() * m_transform;
	else
		return m_transform;
}

Vector2 CharacterInstance::transformInto(const CharacterInstance* other, const Vector2& pnt) const
{
	Vector2 glb = getFullTransform() * pnt;
	return other->getFullTransform().inverse() * glb;
}

void CharacterInstance::setFilter(uint8_t filter)
{
	m_filter = filter;
}

void CharacterInstance::setFilterColor(const Color4f& filterColor)
{
	m_filterColor = filterColor;
}

void CharacterInstance::setBlendMode(uint8_t blendMode)
{
	m_blendMode = blendMode;
}

void CharacterInstance::setVisible(bool visible)
{
	m_visible = visible;
}

void CharacterInstance::setEnabled(bool enabled)
{
	m_enabled = enabled;
}

void CharacterInstance::setFocus()
{
	getContext()->setFocus(this);
}

bool CharacterInstance::haveFocus() const
{
	return bool(getContext()->getFocus() == this);
}

void CharacterInstance::setEvents(const SmallMap< uint32_t, Ref< const IActionVMImage > >& eventScripts)
{
	m_eventScripts = eventScripts;
}

void CharacterInstance::eventInit()
{
	SmallMap< uint32_t, Ref< const IActionVMImage > >::iterator i = m_eventScripts.find(EvtInitialize);
	if (i != m_eventScripts.end())
	{
		ActionObject* self = getAsObject(m_context);
		Ref< ActionObject > super = self->getSuper();

		ActionFrame callFrame(
			m_context,
			self,
			4,
			0,
			0
		);

		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		callFrame.setVariable(ActionContext::IdGlobal, ActionValue(m_context->getGlobal()));

		i->second->execute(&callFrame);
	}
}

void CharacterInstance::eventConstruct()
{
	SmallMap< uint32_t, Ref< const IActionVMImage > >::iterator i = m_eventScripts.find(EvtConstruct);
	if (i != m_eventScripts.end())
	{
		ActionObject* self = getAsObject(m_context);
		Ref< ActionObject > super = self->getSuper();

		ActionFrame callFrame(
			m_context,
			self,
			4,
			0,
			0
		);

		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		callFrame.setVariable(ActionContext::IdGlobal, ActionValue(m_context->getGlobal()));

		i->second->execute(&callFrame);
	}
}

void CharacterInstance::eventLoad()
{
	SmallMap< uint32_t, Ref< const IActionVMImage > >::iterator i = m_eventScripts.find(EvtLoad);
	if (i != m_eventScripts.end())
	{
		ActionObject* self = getAsObject(m_context);
		Ref< ActionObject > super = self->getSuper();

		ActionFrame callFrame(
			m_context,
			self,
			4,
			0,
			0
		);

		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		callFrame.setVariable(ActionContext::IdGlobal, ActionValue(m_context->getGlobal()));

		i->second->execute(&callFrame);
	}
}

void CharacterInstance::eventFrame()
{
	SmallMap< uint32_t, Ref< const IActionVMImage > >::iterator i = m_eventScripts.find(EvtEnterFrame);
	if (i != m_eventScripts.end())
	{
		ActionObject* self = getAsObject(m_context);
		Ref< ActionObject > super = self->getSuper();

		ActionFrame callFrame(
			m_context,
			self,
			4,
			0,
			0
		);

		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		callFrame.setVariable(ActionContext::IdGlobal, ActionValue(m_context->getGlobal()));

		i->second->execute(&callFrame);
	}
}

void CharacterInstance::eventKey(wchar_t unicode)
{
}

void CharacterInstance::eventKeyDown(int keyCode)
{
}

void CharacterInstance::eventKeyUp(int keyCode)
{
}

void CharacterInstance::eventMouseDown(int x, int y, int button)
{
}

void CharacterInstance::eventMouseUp(int x, int y, int button)
{
}

void CharacterInstance::eventMouseMove(int x, int y, int button)
{
}

void CharacterInstance::eventSetFocus()
{
	executeScriptEvent(ActionContext::IdOnSetFocus, ActionValue());
}

void CharacterInstance::eventKillFocus()
{
	executeScriptEvent(ActionContext::IdOnKillFocus, ActionValue());
}

bool CharacterInstance::getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue)
{
	if (m_parent && memberName == ActionContext::IdParent)
	{
		outMemberValue = ActionValue(m_parent->getAsObject(context));
		return true;
	}
	else
		return false;
}

bool CharacterInstance::haveScriptEvent(uint32_t eventName)
{
	if (!m_context)
		return false;

	ActionObject* self = getAsObject(m_context);
	T_ASSERT (self);

	ActionValue memberValue;
	if (!self->getMember(eventName, memberValue))
		return false;

	return memberValue.getObject< ActionFunction >() != 0;
}

bool CharacterInstance::executeScriptEvent(uint32_t eventName, const ActionValue& arg)
{
	if (!m_context)
		return false;

	ActionObject* self = getAsObject(m_context);
	T_ASSERT (self);

	ActionValue memberValue;
	if (!self->getMember(eventName, memberValue))
		return false;

	Ref< ActionFunction > eventFunction = memberValue.getObject< ActionFunction >();
	if (!eventFunction)
		return false;

	ActionValueArray argv(m_context->getPool(), 1);
	argv[0] = arg;

	eventFunction->call(self, argv);
	return true;
}

void CharacterInstance::trace(visitor_t visitor) const
{
	visitor(m_context);
	ActionObjectRelay::trace(visitor);
}

void CharacterInstance::dereference()
{
	m_context = 0;
	m_parent = 0;
	ActionObjectRelay::dereference();
}

	}
}
