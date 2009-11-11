#include "Flash/FlashCharacterInstance.h"
#include "Flash/Action/ActionVM.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionScript.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashCharacterInstance", FlashCharacterInstance, ActionObject)

FlashCharacterInstance::FlashCharacterInstance(ActionContext* context, ActionObject* prototype, FlashCharacterInstance* parent)
:	ActionObject(prototype)
,	m_context(context)
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

Ref< ActionContext > FlashCharacterInstance::getContext() const
{
	return m_context;
}

Ref< FlashCharacterInstance > FlashCharacterInstance::getParent() const
{
	return m_parent;
}

void FlashCharacterInstance::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& FlashCharacterInstance::getName() const
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

void FlashCharacterInstance::setEvent(uint32_t eventMask, ActionScript* eventScript)
{
	m_eventScripts[eventMask] = eventScript;
}

void FlashCharacterInstance::setEvents(const std::map< uint32_t, Ref< ActionScript > >& eventScripts)
{
	m_eventScripts = eventScripts;
}

const std::map< uint32_t, Ref< ActionScript > >& FlashCharacterInstance::getEvents() const
{
	return m_eventScripts;
}

void FlashCharacterInstance::eventInit(ActionVM* actionVM)
{
	std::map< uint32_t, Ref< ActionScript > >::iterator i = m_eventScripts.find(EvtInitialize);
	if (i != m_eventScripts.end())
	{
		ActionFrame callFrame(
			m_context,
			this,
			i->second->getCode(),
			i->second->getCodeSize(),
			4,
			0,
			0
		);
		actionVM->execute(&callFrame);
	}
}

void FlashCharacterInstance::eventLoad(ActionVM* actionVM)
{
	std::map< uint32_t, Ref< ActionScript > >::iterator i = m_eventScripts.find(EvtLoad);
	if (i != m_eventScripts.end())
	{
		ActionFrame callFrame(
			m_context,
			this,
			i->second->getCode(),
			i->second->getCodeSize(),
			4,
			0,
			0
		);
		actionVM->execute(&callFrame);
	}
}

void FlashCharacterInstance::eventFrame(ActionVM* actionVM)
{
	std::map< uint32_t, Ref< ActionScript > >::iterator i = m_eventScripts.find(EvtEnterFrame);
	if (i != m_eventScripts.end())
	{
		ActionFrame callFrame(
			m_context,
			this,
			i->second->getCode(),
			i->second->getCodeSize(),
			4,
			0,
			0
		);
		actionVM->execute(&callFrame);
	}
}

void FlashCharacterInstance::eventKeyDown(ActionVM* actionVM, int keyCode)
{
}

void FlashCharacterInstance::eventKeyUp(ActionVM* actionVM, int keyCode)
{
}

void FlashCharacterInstance::eventMouseDown(ActionVM* actionVM, int x, int y, int button)
{
}

void FlashCharacterInstance::eventMouseUp(ActionVM* actionVM, int x, int y, int button)
{
}

void FlashCharacterInstance::eventMouseMove(ActionVM* actionVM, int x, int y, int button)
{
}

	}
}
