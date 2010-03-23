#include <limits>
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashFrame.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionScript.h"
#include "Flash/Action/IActionVM.h"
#include "Flash/Action/Avm1/Classes/AsMovieClip.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashSpriteInstance", FlashSpriteInstance, FlashCharacterInstance)

FlashSpriteInstance::FlashSpriteInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashSprite* sprite)
:	FlashCharacterInstance(context, AsMovieClip::getInstance(), parent)
,	m_sprite(sprite)
,	m_displayList(context)
,	m_currentFrame(0)
,	m_nextFrame(0)
,	m_lastUpdateFrame(~0U)
,	m_lastExecutedFrame(~0U)
,	m_skipEnterFrame(0)
,	m_initialized(false)
,	m_playing(true)
,	m_visible(true)
,	m_inDispatch(false)
,	m_gotoIssued(false)
,	m_mouseX(0)
,	m_mouseY(0)
{
	T_ASSERT (m_sprite->getFrameCount() > 0);
	updateDisplayList();
}

const FlashSprite* FlashSpriteInstance::getSprite() const
{
	return m_sprite;
}

void FlashSpriteInstance::gotoFrame(uint32_t frameId)
{
	frameId = min(frameId, m_sprite->getFrameCount() - 1);
	if (!m_inDispatch)
	{
		m_currentFrame =
		m_nextFrame = frameId;
	}
	else
	{
		m_nextFrame = frameId;
		m_skipEnterFrame = 1;
		m_gotoIssued = true;
	}
}

void FlashSpriteInstance::gotoPrevious()
{
	if (!m_inDispatch)
	{
		if (m_currentFrame > 0)
			m_currentFrame--;
	}
	else
	{
		if (m_currentFrame > 0)
		{
			m_nextFrame = m_currentFrame - 1;
			m_gotoIssued = true;
		}
	}
}

void FlashSpriteInstance::gotoNext()
{
	if (!m_inDispatch)
	{
		if (m_currentFrame < m_sprite->getFrameCount() - 1)
			m_currentFrame++;
	}
	else
	{
		if (m_currentFrame < m_sprite->getFrameCount() - 1)
		{
			m_nextFrame = m_currentFrame + 1;
			m_gotoIssued = true;
		}
	}
}

uint32_t FlashSpriteInstance::getCurrentFrame() const
{
	return m_currentFrame;
}

void FlashSpriteInstance::setPlaying(bool playing)
{
	m_playing = playing;
}

bool FlashSpriteInstance::getPlaying() const
{
	return m_playing;
}

void FlashSpriteInstance::updateDisplayList()
{
	// Update sprite instance's display list.
	if (m_currentFrame < m_lastUpdateFrame)
	{
		m_displayList.updateBegin(true);
		for (uint32_t i = 0; i <= m_currentFrame; ++i)
		{
			Ref< FlashFrame > frame = m_sprite->getFrame(i);
			T_ASSERT (frame);

			m_displayList.updateFrame(this, frame);
		}
		m_displayList.updateEnd();
	}
	else if (m_currentFrame > m_lastUpdateFrame)
	{
		m_displayList.updateBegin(false);
		for (uint32_t i = m_lastUpdateFrame; i <= m_currentFrame; ++i)
		{
			Ref< FlashFrame > frame = m_sprite->getFrame(i);
			T_ASSERT (frame);

			m_displayList.updateFrame(this, frame);
		}
		m_displayList.updateEnd();
	}

	m_lastUpdateFrame = m_currentFrame;

	// Recursive update of child sprite instances as well.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (is_a< FlashSpriteInstance >(i->second.instance))
			static_cast< FlashSpriteInstance* >(i->second.instance.ptr())->updateDisplayList();
	}
}

void FlashSpriteInstance::preDispatchEvents(IActionVM* actionVM)
{
	T_ASSERT (!m_inDispatch);
	m_inDispatch = true;
	m_gotoIssued = false;

	// Initialize sprite instance.
	if (!m_initialized)
	{
		eventInit(actionVM);
		eventLoad(actionVM);
		m_initialized = true;
	}

	// Set initial next frame index, this might change during execution of events.
	if (m_playing)
		m_nextFrame = (m_currentFrame + 1) % m_sprite->getFrameCount();
	else
		m_nextFrame = m_currentFrame;

	// Issue pre-dispatch event on child instances.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (FlashSpriteInstance* spriteInstance = dynamic_type_cast< FlashSpriteInstance* >(i->second.instance))
			spriteInstance->preDispatchEvents(actionVM);
	}
}

void FlashSpriteInstance::postDispatchEvents(IActionVM* actionVM)
{
	if (!m_inDispatch)
		return;

	// Update current frame index.
	if (m_playing || m_gotoIssued)
	{
		T_ASSERT (m_nextFrame < m_sprite->getFrameCount());
		m_currentFrame = m_nextFrame;
	}

	// Issue post dispatch event on child sprite instances.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (FlashSpriteInstance* spriteInstance = dynamic_type_cast< FlashSpriteInstance* >(i->second.instance))
			spriteInstance->postDispatchEvents(actionVM);
	}

	m_inDispatch = false;
}

FlashDisplayList& FlashSpriteInstance::getDisplayList()
{
	return m_displayList;
}

void FlashSpriteInstance::setVisible(bool visible)
{
	m_visible = visible;
}

bool FlashSpriteInstance::isVisible() const
{
	return m_visible;
}

FlashSpriteInstance* FlashSpriteInstance::clone() const
{
	Ref< FlashSpriteInstance > cloneInstance = checked_type_cast< FlashSpriteInstance* >(m_sprite->createInstance(getContext(), getParent()));
	cloneInstance->setEvents(getEvents());
	return cloneInstance;
}

SwfRect FlashSpriteInstance::getLocalBounds() const
{
	SwfRect bounds =
	{
		Vector2( std::numeric_limits< float >::max(),  std::numeric_limits< float >::max()),
		Vector2(-std::numeric_limits< float >::max(), -std::numeric_limits< float >::max())
	};

	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		SwfRect layerBounds = i->second.instance->getBounds();
		bounds.min.x = std::min(bounds.min.x, layerBounds.min.x);
		bounds.min.y = std::min(bounds.min.y, layerBounds.min.y);
		bounds.max.x = std::max(bounds.max.x, layerBounds.max.x);
		bounds.max.y = std::max(bounds.max.y, layerBounds.max.y);
	}

	return bounds;
}

bool FlashSpriteInstance::getMember(const std::wstring& memberName, ActionValue& outMemberValue) const
{
	// Get names instance from display list.
	FlashDisplayList::layer_map_t::const_iterator i = m_displayList.findLayer(memberName);
	if (i != m_displayList.getLayers().end())
	{
		outMemberValue = ActionValue(i->second.instance);
		return true;
	}

	return FlashCharacterInstance::getMember(memberName, outMemberValue);
}

void FlashSpriteInstance::eventInit(const IActionVM* actionVM)
{
	const RefArray< ActionScript >& initActionScripts = m_sprite->getInitActionScripts();
	for (RefArray< ActionScript >::const_iterator i = initActionScripts.begin(); i != initActionScripts.end(); ++i)
	{
		ActionFrame frame(getContext(), this, (*i)->getCode(), (*i)->getCodeSize(), 4, 0, 0);
		actionVM->execute(&frame);
	}

	FlashCharacterInstance::eventInit(actionVM);
}

void FlashSpriteInstance::eventLoad(const IActionVM* actionVM)
{
	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventLoad(actionVM);
	}

	// Issue script assigned event.
	executeScriptEvent(actionVM, L"onLoad");

	FlashCharacterInstance::eventLoad(actionVM);
}

void FlashSpriteInstance::eventFrame(const IActionVM* actionVM)
{
	Ref< FlashFrame > frame = m_sprite->getFrame(m_currentFrame);
	T_ASSERT (frame);

	// Issue script assigned event; hack to skip events when using goto methods.
	if (!m_skipEnterFrame)
		executeScriptEvent(actionVM, L"onEnterFrame");
	else
		--m_skipEnterFrame;

	// Execute frame scripts.
	if (m_lastExecutedFrame != m_currentFrame)
	{
		const RefArray< ActionScript >& scripts = frame->getActionScripts();
		for (RefArray< ActionScript >::const_iterator i = scripts.begin(); i != scripts.end(); ++i)
		{
			ActionFrame callFrame(getContext(), this, (*i)->getCode(), (*i)->getCodeSize(), 4, 0, 0);
			actionVM->execute(&callFrame);
		}
		m_lastExecutedFrame = m_currentFrame;
	}

	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventFrame(actionVM);
	}

	FlashCharacterInstance::eventFrame(actionVM);
}

void FlashSpriteInstance::eventKeyDown(const IActionVM* actionVM, int keyCode)
{
	// Issue script assigned event.
	executeScriptEvent(actionVM, L"onKeyDown");

	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventKeyDown(actionVM, keyCode);
	}

	FlashCharacterInstance::eventKeyDown(actionVM, keyCode);
}

void FlashSpriteInstance::eventKeyUp(const IActionVM* actionVM, int keyCode)
{
	// Issue script assigned event.
	executeScriptEvent(actionVM, L"onKeyUp");

	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventKeyUp(actionVM, keyCode);
	}

	FlashCharacterInstance::eventKeyUp(actionVM, keyCode);
}

void FlashSpriteInstance::eventMouseDown(const IActionVM* actionVM, int x, int y, int button)
{
	m_mouseX = x;
	m_mouseY = y;

	// Issue script assigned event.
	executeScriptEvent(actionVM, L"onMouseDown");

	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventMouseDown(actionVM, x, y, button);
	}

	FlashCharacterInstance::eventMouseDown(actionVM, x, y, button);
}

void FlashSpriteInstance::eventMouseUp(const IActionVM* actionVM, int x, int y, int button)
{
	m_mouseX = x;
	m_mouseY = y;

	// Issue script assigned event.
	executeScriptEvent(actionVM, L"onMouseUp");

	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventMouseUp(actionVM, x, y, button);
	}

	FlashCharacterInstance::eventMouseUp(actionVM, x, y, button);
}

void FlashSpriteInstance::eventMouseMove(const IActionVM* actionVM, int x, int y, int button)
{
	m_mouseX = x;
	m_mouseY = y;

	// Issue script assigned event.
	executeScriptEvent(actionVM, L"onMouseMove");

	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventMouseMove(actionVM, x, y, button);
	}

	FlashCharacterInstance::eventMouseMove(actionVM, x, y, button);
}

SwfRect FlashSpriteInstance::getBounds() const
{
	SwfRect bounds = getLocalBounds();

	bounds.min = getTransform() * bounds.min;
	bounds.max = getTransform() * bounds.max;

	return bounds;
}

void FlashSpriteInstance::executeScriptEvent(const IActionVM* actionVM, const std::wstring& eventName)
{
	ActionValue memberValue;
	if (!getLocalMember(eventName, memberValue))
		return;

	Ref< ActionFunction > eventFunction = memberValue.getObject< ActionFunction >();
	if (!eventFunction)
		return;

	ActionFrame callFrame(getContext(), this, 0, 0, 4, 0, 0);
	eventFunction->call(actionVM, &callFrame, this);
}

	}
}
