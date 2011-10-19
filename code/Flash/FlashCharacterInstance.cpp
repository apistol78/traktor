#include "Flash/FlashCharacterInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionObject.h"
#include "Flash/Action/IActionVM.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashCharacterInstance", FlashCharacterInstance, ActionObjectRelay)

FlashCharacterInstance::FlashCharacterInstance(ActionContext* context, const std::string& prototype, FlashCharacterInstance* parent)
:	ActionObjectRelay(prototype)
,	m_context(context)
,	m_prototype(prototype)
,	m_parent(parent)
{
	m_cxform.red[0] =
	m_cxform.green[0] =
	m_cxform.blue[0] =
	m_cxform.alpha[0] = 1.0f;
	m_cxform.red[1] =
	m_cxform.green[1] =
	m_cxform.blue[1] =
	m_cxform.alpha[1] = 0.0f;
	m_transform = Matrix33::identity();
}

void FlashCharacterInstance::destroy()
{
	m_context = 0;
	m_parent = 0;
	m_eventScripts.clear();
}

ActionContext* FlashCharacterInstance::getContext() const
{
	return m_context;
}

FlashCharacterInstance* FlashCharacterInstance::getParent() const
{
	return m_parent;
}

void FlashCharacterInstance::setName(const std::string& name)
{
	m_name = name;
}

const std::string& FlashCharacterInstance::getName() const
{
	return m_name;
}

void FlashCharacterInstance::setColorTransform(const SwfCxTransform& cxform)
{
	m_cxform = cxform;
}

const SwfCxTransform& FlashCharacterInstance::getColorTransform() const
{
	return m_cxform;
}

void FlashCharacterInstance::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

const Matrix33& FlashCharacterInstance::getTransform() const
{
	return m_transform;
}

void FlashCharacterInstance::setEvent(uint32_t eventMask, const IActionVMImage* eventScript)
{
	m_eventScripts[eventMask] = eventScript;
}

void FlashCharacterInstance::setEvents(const SmallMap< uint32_t, Ref< const IActionVMImage > >& eventScripts)
{
	m_eventScripts = eventScripts;
}

const SmallMap< uint32_t, Ref< const IActionVMImage > >& FlashCharacterInstance::getEvents() const
{
	return m_eventScripts;
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
		ActionFrame callFrame(
			m_context,
			getAsObject(),
			i->second,
			4,
			0,
			0
		);
		m_context->getVM()->execute(&callFrame);
	}
}

void FlashCharacterInstance::eventLoad()
{
	SmallMap< uint32_t, Ref< const IActionVMImage > >::iterator i = m_eventScripts.find(EvtLoad);
	if (i != m_eventScripts.end())
	{
		ActionFrame callFrame(
			m_context,
			getAsObject(),
			i->second,
			4,
			0,
			0
		);
		m_context->getVM()->execute(&callFrame);
	}
}

void FlashCharacterInstance::eventFrame()
{
	SmallMap< uint32_t, Ref< const IActionVMImage > >::iterator i = m_eventScripts.find(EvtEnterFrame);
	if (i != m_eventScripts.end())
	{
		ActionFrame callFrame(
			m_context,
			getAsObject(),
			i->second,
			4,
			0,
			0
		);
		m_context->getVM()->execute(&callFrame);
	}
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

void FlashCharacterInstance::eventMouseMove(int x, int y, int button)
{
}

void FlashCharacterInstance::trace(const IVisitor& visitor) const
{
	visitor(m_context);
	ActionObjectRelay::trace(visitor);
}

void FlashCharacterInstance::dereference()
{
	m_context = 0;
	ActionObjectRelay::dereference();
}

	}
}
