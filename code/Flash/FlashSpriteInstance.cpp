#include <limits>
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashFrame.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionScript.h"
#include "Flash/Action/IActionVM.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashSpriteInstance", FlashSpriteInstance, FlashCharacterInstance)

FlashSpriteInstance::FlashSpriteInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashSprite* sprite)
:	FlashCharacterInstance(context, "MovieClip", parent)
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
,	m_maskCount(0)
{
	T_ASSERT (m_sprite->getFrameCount() > 0);
	updateDisplayList();
}

void FlashSpriteInstance::destroy()
{
	m_sprite = 0;
	m_mask = 0;

	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->destroy();
	}

	m_displayList.reset();

	FlashCharacterInstance::destroy();
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
	return m_visible && m_maskCount == 0;
}

Ref< FlashSpriteInstance > FlashSpriteInstance::clone() const
{
	Ref< FlashSpriteInstance > cloneInstance = checked_type_cast< FlashSpriteInstance* >(m_sprite->createInstance(getContext(), getParent(), ""));
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

void FlashSpriteInstance::setMask(FlashSpriteInstance* mask)
{
	if (m_mask)
		m_mask->m_maskCount--;
	if ((m_mask = mask) != 0)
		m_mask->m_maskCount++;
}

FlashSpriteInstance* FlashSpriteInstance::getMask()
{
	return m_mask;
}

bool FlashSpriteInstance::getMember(ActionContext* context, const std::string& memberName, ActionValue& outMemberValue)
{
	// Get names instance from display list.
	FlashDisplayList::layer_map_t::const_iterator i = m_displayList.findLayer(memberName);
	if (i != m_displayList.getLayers().end())
	{
		outMemberValue = ActionValue(i->second.instance);
		return true;
	}

	return FlashCharacterInstance::getMember(context, memberName, outMemberValue);
}

void FlashSpriteInstance::preDispatchEvents()
{
	T_ASSERT (!m_inDispatch);
	m_inDispatch = true;
	m_gotoIssued = false;

	// Initialize sprite instance.
	if (!m_initialized)
	{
		eventInit();
		eventLoad();
		m_initialized = true;
	}

	// Set initial next frame index, this might change during execution of events.
	if (m_playing)
		m_nextFrame = (m_currentFrame + 1) % m_sprite->getFrameCount();
	else
		m_nextFrame = m_currentFrame;

	// Issue dispatch event on child instances.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->preDispatchEvents();
	}
}

void FlashSpriteInstance::postDispatchEvents()
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
		if (i->second.instance)
			i->second.instance->postDispatchEvents();
	}

	m_inDispatch = false;
}

void FlashSpriteInstance::eventInit()
{
	ActionContext* context = getContext();

	const RefArray< ActionScript >& initActionScripts = m_sprite->getInitActionScripts();
	for (RefArray< ActionScript >::const_iterator i = initActionScripts.begin(); i != initActionScripts.end(); ++i)
	{
		ActionFrame frame(context, this, (*i)->getCode(), (*i)->getCodeSize(), 4, 0, 0);
		context->getVM()->execute(&frame);
	}

	FlashCharacterInstance::eventInit();
}

void FlashSpriteInstance::eventLoad()
{
	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventLoad();
	}

	// Issue script assigned event.
	executeScriptEvent("onLoad");

	FlashCharacterInstance::eventLoad();
}

void FlashSpriteInstance::eventFrame()
{
	Ref< FlashFrame > frame = m_sprite->getFrame(m_currentFrame);
	T_ASSERT (frame);

	// Issue script assigned event; hack to skip events when using goto methods.
	if (!m_skipEnterFrame)
		executeScriptEvent("onEnterFrame");
	else
		--m_skipEnterFrame;

	// Execute frame scripts.
	if (m_lastExecutedFrame != m_currentFrame)
	{
		ActionContext* context = getContext();

		const RefArray< ActionScript >& scripts = frame->getActionScripts();
		for (RefArray< ActionScript >::const_iterator i = scripts.begin(); i != scripts.end(); ++i)
		{
			ActionFrame callFrame(context, this, (*i)->getCode(), (*i)->getCodeSize(), 4, 0, 0);
			context->getVM()->execute(&callFrame);
		}

		m_lastExecutedFrame = m_currentFrame;
	}

	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventFrame();
	}

	FlashCharacterInstance::eventFrame();
}

void FlashSpriteInstance::eventKeyDown(int32_t keyCode)
{
	// Issue script assigned event.
	executeScriptEvent("onKeyDown");

	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventKeyDown(keyCode);
	}

	FlashCharacterInstance::eventKeyDown(keyCode);
}

void FlashSpriteInstance::eventKeyUp(int32_t keyCode)
{
	// Issue script assigned event.
	executeScriptEvent("onKeyUp");

	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventKeyUp(keyCode);
	}

	FlashCharacterInstance::eventKeyUp(keyCode);
}

void FlashSpriteInstance::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
	m_mouseX = x;
	m_mouseY = y;

	// Issue script assigned event.
	executeScriptEvent("onMouseDown");

	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventMouseDown(x, y, button);
	}

	FlashCharacterInstance::eventMouseDown(x, y, button);
}

void FlashSpriteInstance::eventMouseUp(int32_t x, int32_t y, int32_t button)
{
	m_mouseX = x;
	m_mouseY = y;

	// Issue script assigned event.
	executeScriptEvent("onMouseUp");

	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventMouseUp(x, y, button);
	}

	FlashCharacterInstance::eventMouseUp(x, y, button);
}

void FlashSpriteInstance::eventMouseMove(int32_t x, int32_t y, int32_t button)
{
	m_mouseX = x;
	m_mouseY = y;

	// Issue script assigned event.
	executeScriptEvent("onMouseMove");

	// Issue events on "visible" characters.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->eventMouseMove(x, y, button);
	}

	FlashCharacterInstance::eventMouseMove(x, y, button);
}

SwfRect FlashSpriteInstance::getBounds() const
{
	SwfRect bounds = getLocalBounds();

	bounds.min = getTransform() * bounds.min;
	bounds.max = getTransform() * bounds.max;

	return bounds;
}

void FlashSpriteInstance::trace(const IVisitor& visitor) const
{
	visitor(m_mask);

	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		visitor(i->second.instance);

	FlashCharacterInstance::trace(visitor);
}

void FlashSpriteInstance::dereference()
{
	m_mask = 0;
	m_displayList.reset();
	FlashCharacterInstance::dereference();
}

void FlashSpriteInstance::executeScriptEvent(const std::string& eventName)
{
	ActionValue memberValue;
	if (!getLocalMember(eventName, memberValue))
		return;

	Ref< ActionFunction > eventFunction = memberValue.getObject< ActionFunction >();
	if (!eventFunction)
		return;

	ActionFrame callFrame(getContext(), this, 0, 0, 4, 0, 0);
	eventFunction->call(&callFrame, this);
}

	}
}
