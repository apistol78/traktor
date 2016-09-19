#include "Flash/FlashCharacterInstance.h"
#include "Flash/FlashTypes.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashCharacterInstance", FlashCharacterInstance, ActionObjectRelay)

int32_t FlashCharacterInstance::ms_instanceCount = 0;

FlashCharacterInstance::FlashCharacterInstance(
	ActionContext* context,
	const char* const prototype,
	FlashDictionary* dictionary,
	FlashCharacterInstance* parent
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

FlashCharacterInstance::~FlashCharacterInstance()
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

int32_t FlashCharacterInstance::getInstanceCount()
{
	return ms_instanceCount;
}

void FlashCharacterInstance::destroy()
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
}

void FlashCharacterInstance::setName(const std::string& name)
{
	m_name = name;
}

const std::string& FlashCharacterInstance::getName() const
{
	return m_name;
}

void FlashCharacterInstance::setCacheObject(IRefCount* cacheObject)
{
	m_cacheObject = cacheObject;
}

void FlashCharacterInstance::clearCacheObject()
{
	m_cacheObject = 0;
}

std::string FlashCharacterInstance::getTarget() const
{
	return m_parent ? (m_parent->getTarget() + "/" + getName()) : "";
}

void FlashCharacterInstance::setColorTransform(const SwfCxTransform& cxform)
{
	clearCacheObject();
	m_cxform = cxform;
}

void FlashCharacterInstance::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

Matrix33 FlashCharacterInstance::getFullTransform() const
{
	if (m_parent)
		return m_parent->getFullTransform() * m_transform;
	else
		return m_transform;
}

Vector2 FlashCharacterInstance::transformInto(const FlashCharacterInstance* other, const Vector2& pnt) const
{
	Vector2 glb = getFullTransform() * pnt;
	return other->getFullTransform().inverse() * glb;
}

void FlashCharacterInstance::setFilter(uint8_t filter)
{
	m_filter = filter;
}

void FlashCharacterInstance::setFilterColor(const Color4f& filterColor)
{
	m_filterColor = filterColor;
}

void FlashCharacterInstance::setBlendMode(uint8_t blendMode)
{
	m_blendMode = blendMode;
}

void FlashCharacterInstance::setVisible(bool visible)
{
	m_visible = visible;
}

void FlashCharacterInstance::setEnabled(bool enabled)
{
	m_enabled = enabled;
}

void FlashCharacterInstance::setEvents(const SmallMap< uint32_t, Ref< const IActionVMImage > >& eventScripts)
{
	m_eventScripts = eventScripts;
}

void FlashCharacterInstance::preDispatchEvents()
{
}

void FlashCharacterInstance::postDispatchEvents()
{
}

void FlashCharacterInstance::eventInit()
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

void FlashCharacterInstance::eventConstruct()
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

void FlashCharacterInstance::eventLoad()
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

void FlashCharacterInstance::eventFrame()
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

void FlashCharacterInstance::eventKey(wchar_t unicode)
{
}

void FlashCharacterInstance::eventKeyDown(int keyCode)
{
}

void FlashCharacterInstance::eventKeyUp(int keyCode)
{
}

void FlashCharacterInstance::eventMouseDown(int x, int y, int button)
{
}

void FlashCharacterInstance::eventMouseUp(int x, int y, int button)
{
}

void FlashCharacterInstance::eventMouseMove0(int x, int y, int button)
{
}

void FlashCharacterInstance::eventMouseMove1(int x, int y, int button)
{
}

void FlashCharacterInstance::eventSetFocus()
{
	executeScriptEvent(ActionContext::IdOnSetFocus, ActionValue());
}

void FlashCharacterInstance::eventKillFocus()
{
	executeScriptEvent(ActionContext::IdOnKillFocus, ActionValue());
}

bool FlashCharacterInstance::getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue)
{
	if (m_parent && memberName == ActionContext::IdParent)
	{
		outMemberValue = ActionValue(m_parent->getAsObject(context));
		return true;
	}
	else
		return false;
}

void FlashCharacterInstance::setParent(FlashCharacterInstance* parent)
{
	m_parent = parent;
}

bool FlashCharacterInstance::haveScriptEvent(uint32_t eventName)
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

bool FlashCharacterInstance::executeScriptEvent(uint32_t eventName, const ActionValue& arg)
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

void FlashCharacterInstance::trace(visitor_t visitor) const
{
	visitor(m_context);
	ActionObjectRelay::trace(visitor);
}

void FlashCharacterInstance::dereference()
{
	m_context = 0;
	m_parent = 0;
	ActionObjectRelay::dereference();
}

	}
}
