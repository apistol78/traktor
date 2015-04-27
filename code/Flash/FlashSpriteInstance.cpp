#include <limits>
#include "Core/Math/Format.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashCanvas.h"
#include "Flash/FlashSound.h"
#include "Flash/FlashSoundPlayer.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashFrame.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
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
,	m_lastSoundFrame(~0U)
,	m_skipEnterFrame(0)
,	m_initialized(false)
,	m_playing(true)
,	m_visible(false)
,	m_enabled(true)
,	m_inside(false)
,	m_inDispatch(false)
,	m_gotoIssued(false)
,	m_mouseX(0)
,	m_mouseY(0)
,	m_maskCount(0)
{
	T_ASSERT (m_sprite->getFrameCount() > 0);
}

FlashSpriteInstance::~FlashSpriteInstance()
{
	destroy();
}

void FlashSpriteInstance::destroy()
{
	m_sprite = 0;
	m_mask = 0;
	m_canvas = 0;
	m_playing = false;

	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->destroy();
	}
	m_displayList.reset();
	m_visibleCharacters.clear();

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
			FlashFrame* frame = m_sprite->getFrame(i);
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
			FlashFrame* frame = m_sprite->getFrame(i);
			T_ASSERT (frame);

			m_displayList.updateFrame(this, frame);
		}
		m_displayList.updateEnd();
	}

	m_lastUpdateFrame = m_currentFrame;

	// Recursive update of child sprite instances as well.
	m_displayList.getVisibleObjects(m_visibleCharacters);
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
	{
		if (&type_of(*i) == &type_of< FlashSpriteInstance >())
			static_cast< FlashSpriteInstance* >(*i)->updateDisplayList();
	}
	m_visibleCharacters.resize(0);
}

FlashDisplayList& FlashSpriteInstance::getDisplayList()
{
	return m_displayList;
}

const FlashDisplayList& FlashSpriteInstance::getDisplayList() const
{
	return m_displayList;
}

void FlashSpriteInstance::updateSounds(FlashSoundPlayer* soundPlayer)
{
	if (m_lastSoundFrame != m_currentFrame)
	{
		FlashFrame* frame = m_sprite->getFrame(m_currentFrame);
		if (frame)
		{
			const AlignedVector< uint16_t >& startSounds = frame->getStartSounds();
			for (AlignedVector< uint16_t >::const_iterator i = startSounds.begin(); i != startSounds.end(); ++i)
			{
				const FlashSound* sound = getContext()->getDictionary()->getSound(*i);
				if (sound)
					soundPlayer->play(sound);
			}
		}
		m_lastSoundFrame = m_currentFrame;
	}

	m_displayList.getVisibleObjects(m_visibleCharacters);
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
	{
		if (&type_of(*i) == &type_of< FlashSpriteInstance >())
			static_cast< FlashSpriteInstance* >(*i)->updateSounds(soundPlayer);
	}
	m_visibleCharacters.resize(0);
}

void FlashSpriteInstance::removeMovieClip()
{
	if (!getParent())
		return;

	ActionContext* context = getContext();
	if (context)
	{
		if (context->getFocus() == this)
			context->setFocus(0);
	}

	Ref< FlashSpriteInstance > parentClipInstance = checked_type_cast< FlashSpriteInstance*, false >(getParent());

	FlashDisplayList& parentDisplayList = parentClipInstance->getDisplayList();
	parentDisplayList.removeObject(this);

	if (parentClipInstance->m_mask == this)
		parentClipInstance->m_mask = 0;

	m_displayList.reset();
	m_mask = 0;
	m_canvas = 0;

	setParent(0);
}

Ref< FlashSpriteInstance > FlashSpriteInstance::clone() const
{
	const SmallMap< uint32_t, Ref< const IActionVMImage > >& events = getEvents();
	Ref< FlashSpriteInstance > cloneInstance = checked_type_cast< FlashSpriteInstance* >(m_sprite->createInstance(
		getContext(),
		getParent(),
		"",
		getTransform(),
		0,
		&events
	));
	return cloneInstance;
}

Aabb2 FlashSpriteInstance::getLocalBounds() const
{
	Aabb2 bounds;

	if (m_canvas)
		bounds = m_canvas->getBounds();

	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		T_ASSERT (i->second.instance);
		bounds.contain(i->second.instance->getBounds());
	}

	return bounds;
}

Aabb2 FlashSpriteInstance::getVisibleLocalBounds() const
{
	Aabb2 bounds;

	if (m_canvas)
		bounds = m_canvas->getBounds();

	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		T_ASSERT (i->second.instance);
		if (i->second.instance->isVisible())
			bounds.contain(i->second.instance->getBounds());
	}

	return bounds;
}

void FlashSpriteInstance::setMask(FlashSpriteInstance* mask)
{
	if (m_mask)
		m_mask->m_maskCount--;
	if ((m_mask = mask) != 0)
		m_mask->m_maskCount++;
	if (m_mask)
		m_mask->setVisible(false);
}

FlashSpriteInstance* FlashSpriteInstance::getMask()
{
	return m_mask;
}

FlashCanvas* FlashSpriteInstance::createCanvas()
{
	if (!m_canvas)
		m_canvas = new FlashCanvas();
	return m_canvas;
}

bool FlashSpriteInstance::enumerateMembers(std::vector< uint32_t >& outMemberNames) const
{
	// Visible named character in display list.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		outMemberNames.push_back(i->second.name);
	return true;
}

bool FlashSpriteInstance::getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue)
{
	// Find visible named character in display list.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.name == memberName)
		{
			outMemberValue = ActionValue(i->second.instance->getAsObject(context));
			return true;
		}
	}

	// No character, propagate to base class.
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
		eventLoad();
		m_initialized = true;
	}

	// Set initial next frame index, this might change during execution of events.
	if (m_playing)
		m_nextFrame = (m_currentFrame + 1) % m_sprite->getFrameCount();
	else
		m_nextFrame = m_currentFrame;

	// Issue dispatch event on visible child instances.
	m_displayList.getVisibleObjects(m_visibleCharacters);
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		(*i)->preDispatchEvents();
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
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		(*i)->postDispatchEvents();
	m_visibleCharacters.resize(0);

	m_inDispatch = false;
}

void FlashSpriteInstance::eventInit()
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	ActionObject* self = getAsObject(context);
	T_ASSERT (self);

	Ref< ActionObject > super = self->getSuper();

	const RefArray< const IActionVMImage >& initActionScripts = m_sprite->getInitActionScripts();
	for (RefArray< const IActionVMImage >::const_iterator i = initActionScripts.begin(); i != initActionScripts.end(); ++i)
	{
		ActionFrame callFrame(
			context,
			self,
			*i,
			4,
			0,
			0
		);

		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		callFrame.setVariable(ActionContext::IdGlobal, ActionValue(context->getGlobal()));

		context->getVM()->execute(&callFrame);
	}

	FlashCharacterInstance::eventInit();

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventConstruct()
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	FlashCharacterInstance::eventConstruct();

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventLoad()
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnLoad, ActionValue());

	FlashCharacterInstance::eventLoad();

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventFrame()
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	FlashFrame* frame = m_sprite->getFrame(m_currentFrame);
	T_ASSERT (frame);

	// Issue script assigned event; hack to skip events when using goto methods.
	if (!m_skipEnterFrame)
		executeScriptEvent(ActionContext::IdOnEnterFrame, ActionValue());
	else
		--m_skipEnterFrame;

	// Execute frame scripts.
	if (m_lastExecutedFrame != m_currentFrame)
	{
		const RefArray< const IActionVMImage >& scripts = frame->getActionScripts();
		if (!scripts.empty())
		{
			ActionObject* self = getAsObject(context);
			T_ASSERT (self);

			Ref< ActionObject > super = self->getSuper();
			for (RefArray< const IActionVMImage >::const_iterator i = scripts.begin(); i != scripts.end(); ++i)
			{
				ActionFrame callFrame(
					context,
					self,
					*i,
					4,
					0,
					0
				);

				callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
				callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
				callFrame.setVariable(ActionContext::IdGlobal, ActionValue(context->getGlobal()));

				context->getVM()->execute(&callFrame);
			}
		}
		m_lastExecutedFrame = m_currentFrame;
	}

	// Issue events on "visible" characters.
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		(*i)->eventFrame();

	FlashCharacterInstance::eventFrame();

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventKey(wchar_t unicode)
{
	// Issue events on "visible" characters.
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		(*i)->eventKey(unicode);

	FlashCharacterInstance::eventKey(unicode);
}

void FlashSpriteInstance::eventKeyDown(int32_t keyCode)
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue events on "visible" characters.
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		(*i)->eventKeyDown(keyCode);

	// Issue script assigned event.
	if (context->getFocus() == this)
		executeScriptEvent(ActionContext::IdOnKeyDown, ActionValue());

	FlashCharacterInstance::eventKeyDown(keyCode);

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventKeyUp(int32_t keyCode)
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue events on "visible" characters.
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		(*i)->eventKeyUp(keyCode);

	// Issue script assigned event.
	if (context->getFocus() == this)
		executeScriptEvent(ActionContext::IdOnKeyUp, ActionValue());

	FlashCharacterInstance::eventKeyUp(keyCode);

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Issue events on "visible" characters.
	if (!m_visibleCharacters.empty())
	{
		for (int32_t i = int32_t(m_visibleCharacters.size() - 1); i >= 0; --i)
			m_visibleCharacters[i]->eventMouseDown(x, y, button);
	}

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnMouseDown, ActionValue());

	// Check if we're inside then issue press events.
	if (!context->getPressed() && isEnabled())
	{
		Aabb2 bounds = getVisibleLocalBounds();
		bool inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
		if (inside)
		{
			if (haveScriptEvent(ActionContext::IdOnPress) || haveScriptEvent(ActionContext::IdOnRelease))
			{
				executeScriptEvent(ActionContext::IdOnPress, ActionValue());
				context->setPressed(this);
			}
		}
	}

	// Call base class event function.
	FlashCharacterInstance::eventMouseDown(x, y, button);

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventMouseUp(int32_t x, int32_t y, int32_t button)
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Issue events on "visible" characters.
	if (!m_visibleCharacters.empty())
	{
		for (int32_t i = int32_t(m_visibleCharacters.size() - 1); i >= 0; --i)
			m_visibleCharacters[i]->eventMouseUp(x, y, button);
	}

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnMouseUp, ActionValue());

	// Check if we're inside then issue press events.
	if (context->getPressed() == this && isEnabled())
	{
		Aabb2 bounds = getVisibleLocalBounds();
		bool inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
		if (inside)
			executeScriptEvent(ActionContext::IdOnRelease, ActionValue());
	}

	FlashCharacterInstance::eventMouseUp(x, y, button);

	context->setMovieClip(current);
	
	// Finally if mouse up has been issued and we're at the bottom
	// of event chain, we drop reference to pressed character.
	if (!getParent())
		context->setPressed(0);
}

void FlashSpriteInstance::eventMouseMove0(int32_t x, int32_t y, int32_t button)
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Issue events on "visible" characters.
	if (!m_visibleCharacters.empty())
	{
		for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
			(*i)->eventMouseMove0(x, y, button);
	}

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnMouseMove, ActionValue());

	// Roll over and out event handling.
	Aabb2 bounds = getVisibleLocalBounds();
	bool inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
	if (inside != m_inside)
	{
		if (!inside)
			executeScriptEvent(ActionContext::IdOnRollOut, ActionValue());
	}

	FlashCharacterInstance::eventMouseMove0(x, y, button);

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventMouseMove1(int32_t x, int32_t y, int32_t button)
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Roll over and out event handling.
	Aabb2 bounds = getVisibleLocalBounds();
	bool inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
	if (inside != m_inside)
	{
		if (inside)
			executeScriptEvent(ActionContext::IdOnRollOver, ActionValue());

		m_inside = inside;
	}

	// Issue events on "visible" characters.
	if (!m_visibleCharacters.empty())
	{
		for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
			(*i)->eventMouseMove1(x, y, button);
	}

	FlashCharacterInstance::eventMouseMove1(x, y, button);

	context->setMovieClip(current);
}

Aabb2 FlashSpriteInstance::getBounds() const
{
	Aabb2 bounds = getLocalBounds();

	Matrix33 transform = getTransform();
	bounds.mn = transform * bounds.mn;
	bounds.mx = transform * bounds.mx;

	return bounds;
}

void FlashSpriteInstance::trace(visitor_t visitor) const
{
	visitor(m_mask);

	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		visitor(*i);

	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		visitor(i->second.instance);

	FlashCharacterInstance::trace(visitor);
}

void FlashSpriteInstance::dereference()
{
	m_mask = 0;
	m_visibleCharacters.resize(0);
	m_displayList.reset();

	FlashCharacterInstance::dereference();
}

	}
}
