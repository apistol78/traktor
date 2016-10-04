#include <limits>
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashCanvas.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashSound.h"
#include "Flash/FlashSoundPlayer.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashSpriteInstance", FlashSpriteInstance, FlashCharacterInstance)

FlashSpriteInstance::FlashSpriteInstance(ActionContext* context, FlashDictionary* dictionary, FlashCharacterInstance* parent, const FlashSprite* sprite)
:	FlashCharacterInstance(context, "MovieClip", dictionary, parent)
,	m_sprite(sprite)
,	m_displayList(context)
,	m_mouseX(0)
,	m_mouseY(0)
,	m_currentFrame(0)
,	m_nextFrame(0)
,	m_lastUpdateFrame(~0)
,	m_lastExecutedFrame(~0)
,	m_lastSoundFrame(~0)
,	m_skipEnterFrame(0)
,	m_cacheAsBitmap(false)
,	m_opaqueBackground(false)
,	m_initialized(false)
,	m_playing(true)
,	m_visible(false)
,	m_enabled(true)
,	m_inside(false)
,	m_inDispatch(false)
,	m_gotoIssued(false)
{
	T_ASSERT (m_sprite->getFrameCount() > 0);
}

FlashSpriteInstance::~FlashSpriteInstance()
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

	FlashCharacterInstance::destroy();
}

void FlashSpriteInstance::setCacheAsBitmap(bool cacheAsBitmap)
{
	m_cacheAsBitmap = cacheAsBitmap;
}

void FlashSpriteInstance::setOpaqueBackground(bool opaqueBackground)
{
	m_opaqueBackground = opaqueBackground;
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

void FlashSpriteInstance::setPlaying(bool playing)
{
	m_playing = playing;
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
			if (frame)
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
			if (frame)
				m_displayList.updateFrame(this, frame);
		}
		m_displayList.updateEnd();
	}
	m_lastUpdateFrame = m_currentFrame;

	m_displayList.forEachVisibleObject([] (FlashCharacterInstance* instance) {
		if (&type_of(instance) == &type_of< FlashSpriteInstance >())
			static_cast< FlashSpriteInstance* >(instance)->updateDisplayList();
	});
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
				const FlashSound* sound = getDictionary()->getSound(*i);
				if (sound)
					soundPlayer->play(sound);
			}
		}
		m_lastSoundFrame = m_currentFrame;
	}

	m_displayList.forEachVisibleObject([&] (FlashCharacterInstance* instance) {
		if (&type_of(instance) == &type_of< FlashSpriteInstance >())
			static_cast< FlashSpriteInstance* >(instance)->updateSounds(soundPlayer);
	});
}

Ref< FlashSpriteInstance > FlashSpriteInstance::createEmptyMovieClip(const std::string& clipName, int32_t depth)
{
	ActionContext* context = getContext();
	T_ASSERT (context);

	// Create fake character ID.
	uint16_t emptyClipId = depth + 40000;

	// Create empty movie character with a single frame.
	Ref< FlashSprite > emptyClip = new FlashSprite(emptyClipId, 0);
	emptyClip->addFrame(new FlashFrame());

	// Create new instance of movie clip.
	Ref< FlashSpriteInstance > emptyClipInstance = checked_type_cast< FlashSpriteInstance* >(emptyClip->createInstance(
		context,
		getDictionary(),
		this,
		clipName,
		Matrix33::identity(),
		0,
		0
	));
	emptyClipInstance->setName(clipName);

	// Add new instance to display list.
	m_displayList.showObject(depth, emptyClipId, emptyClipInstance, true);
	return emptyClipInstance;
}

Ref< FlashEditInstance > FlashSpriteInstance::createTextField(const std::string& textName, int32_t depth, float x, float y, float width, float height)
{
	ActionContext* context = getContext();
	T_ASSERT (context);

	// Get dictionary.
	FlashDictionary* dictionary = getDictionary();
	if (!dictionary)
		return 0;

	Aabb2 bounds(
		Vector2(0.0f, 0.0f),
		Vector2(width, height)
	);
	Color4f color(0.0f, 0.0f, 0.0f, 0.0f);

	// Create edit character.
	Ref< FlashEdit > edit = new FlashEdit(
		-1,
		0,
		12,
		bounds,
		color,
		std::numeric_limits< uint16_t >::max(),
		L"",
		StaLeft,
		0,
		0,
		0,
		0,
		true,
		false,
		false,
		false,
		false
	);

	// Create edit character instance.
	Ref< FlashEditInstance > editInstance = checked_type_cast< FlashEditInstance*, false >(edit->createInstance(
		context,
		dictionary,
		this,
		textName,
		Matrix33::identity(),
		0,
		0
	));
	
	// Place character at given location.
	editInstance->setTransform(translate(x, y));
	
	// Show edit character instance.
	getDisplayList().showObject(
		depth,
		edit->getId(),
		editInstance,
		true
	);

	return editInstance;
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

	setCacheObject(0);
	setParent(0);
}

Ref< FlashSpriteInstance > FlashSpriteInstance::clone() const
{
	const SmallMap< uint32_t, Ref< const IActionVMImage > >& events = getEvents();
	Ref< FlashSpriteInstance > cloneInstance = checked_type_cast< FlashSpriteInstance* >(m_sprite->createInstance(
		getContext(),
		getDictionary(),
		getParent(),
		"",
		getTransform(),
		0,
		&events
	));
	return cloneInstance;
}

Ref< FlashSpriteInstance > FlashSpriteInstance::duplicateMovieClip(const std::string& cloneName, int32_t depth)
{
	FlashSpriteInstance* parent = mandatory_non_null_type_cast< FlashSpriteInstance* >(getParent());
	return duplicateMovieClip(cloneName, depth, parent);
}

Ref< FlashSpriteInstance > FlashSpriteInstance::duplicateMovieClip(const std::string& cloneName, int32_t depth, FlashSpriteInstance* intoParent)
{
	const SmallMap< uint32_t, Ref< const IActionVMImage > >& events = getEvents();

	Ref< FlashSpriteInstance > cloneInstance = checked_type_cast< FlashSpriteInstance* >(m_sprite->createInstance(
		getContext(),
		getDictionary(),
		intoParent,
		cloneName,
		getTransform(),
		0,
		&events
	));

	intoParent->getDisplayList().showObject(
		depth,
		cloneInstance->getSprite()->getId(),
		cloneInstance,
		true
	);

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
	clearCacheObject();
	if ((m_mask = mask) != 0)
		m_mask->setVisible(false);
}

FlashCanvas* FlashSpriteInstance::createCanvas()
{
	clearCacheObject();
	if (!m_canvas)
		m_canvas = new FlashCanvas();
	return m_canvas;
}

void FlashSpriteInstance::clearCacheObject()
{
	FlashCharacterInstance::clearCacheObject();
	m_displayList.forEachVisibleObject([] (FlashCharacterInstance* instance) {
		instance->clearCacheObject();
	});
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
	if (m_inDispatch)
		return;

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
	m_displayList.forEachVisibleObject([] (FlashCharacterInstance* instance) {
		instance->preDispatchEvents();
	});
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
	m_displayList.forEachVisibleObject([] (FlashCharacterInstance* instance) {
		instance->postDispatchEvents();
	});

	m_inDispatch = false;
}

void FlashSpriteInstance::eventInit()
{
	ActionContext* context = getContext();
	if (!context)
		return;

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
			4,
			0,
			0
		);

		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		callFrame.setVariable(ActionContext::IdGlobal, ActionValue(context->getGlobal()));

		(*i)->execute(&callFrame);
	}

	FlashCharacterInstance::eventInit();

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventConstruct()
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	FlashCharacterInstance::eventConstruct();

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventLoad()
{
	ActionContext* context = getContext();
	if (!context)
		return;

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
	if (!context)
		return;

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue script assigned event; hack to skip events when using goto methods.
	if (!m_skipEnterFrame)
		executeScriptEvent(ActionContext::IdOnEnterFrame, ActionValue());
	else
		--m_skipEnterFrame;

	// Execute frame scripts.
	if (m_lastExecutedFrame != m_currentFrame)
	{
		FlashFrame* frame = m_sprite->getFrame(m_currentFrame);
		T_ASSERT (frame);

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
					4,
					0,
					0
				);

				callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
				callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
				callFrame.setVariable(ActionContext::IdGlobal, ActionValue(context->getGlobal()));

				callFrame.setRegister(0, ActionValue(context->getGlobal()));

				(*i)->execute(&callFrame);
			}
		}
		m_lastExecutedFrame = m_currentFrame;
	}

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObject([] (FlashCharacterInstance* instance) {
		instance->eventFrame();
	});

	FlashCharacterInstance::eventFrame();

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventKey(wchar_t unicode)
{
	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObject([&] (FlashCharacterInstance* instance) {
		instance->eventKey(unicode);
	});

	FlashCharacterInstance::eventKey(unicode);
}

void FlashSpriteInstance::eventKeyDown(int32_t keyCode)
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObject([&] (FlashCharacterInstance* instance) {
		instance->eventKeyDown(keyCode);
	});

	// Issue script assigned event.
	if (context->getFocus() == this)
		executeScriptEvent(ActionContext::IdOnKeyDown, ActionValue());

	FlashCharacterInstance::eventKeyDown(keyCode);

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventKeyUp(int32_t keyCode)
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObject([&] (FlashCharacterInstance* instance) {
		instance->eventKeyUp(keyCode);
	});

	// Issue script assigned event.
	if (context->getFocus() == this)
		executeScriptEvent(ActionContext::IdOnKeyUp, ActionValue());

	FlashCharacterInstance::eventKeyUp(keyCode);

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Remove focus item if we're root, focus might be shifted.
	if (!getParent())
		context->setFocus(0);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObjectReverse([&] (FlashCharacterInstance* instance) {
		instance->eventMouseDown(x, y, button);
	});

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnMouseDown, ActionValue());

	// Check if we're inside then issue press events.
	if (!context->getPressed() && isEnabled())
	{
		Aabb2 bounds = getVisibleLocalBounds();
		if (bounds.inside(xy))
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
	if (!context)
		return;

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObjectReverse([&] (FlashCharacterInstance* instance) {
		instance->eventMouseUp(x, y, button);
	});

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
	if (!context)
		return;

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObject([&] (FlashCharacterInstance* instance) {
		instance->eventMouseMove0(x, y, button);
	});

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnMouseMove, ActionValue());

	// Roll over and out event handling.
	if (haveScriptEvent(ActionContext::IdOnRollOut))
	{
		Aabb2 bounds = getVisibleLocalBounds();
		bool inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
		if (inside != m_inside)
		{
			if (!inside)
				executeScriptEvent(ActionContext::IdOnRollOut, ActionValue());
		}
	}

	FlashCharacterInstance::eventMouseMove0(x, y, button);

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventMouseMove1(int32_t x, int32_t y, int32_t button)
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Roll over and out event handling.
	if (haveScriptEvent(ActionContext::IdOnRollOver))
	{
		Aabb2 bounds = getVisibleLocalBounds();
		bool inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
		if (inside != m_inside)
		{
			if (inside)
				executeScriptEvent(ActionContext::IdOnRollOver, ActionValue());

			m_inside = inside;
		}
	}

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObject([&] (FlashCharacterInstance* instance) {
		instance->eventMouseMove1(x, y, button);
	});

	FlashCharacterInstance::eventMouseMove1(x, y, button);

	context->setMovieClip(current);
}

Aabb2 FlashSpriteInstance::getBounds() const
{
	return getTransform() * getLocalBounds();
}

void FlashSpriteInstance::setPosition(const Vector2& position)
{
	Matrix33 m = getTransform();
	m.e13 = position.x * 20.0f;
	m.e23 = position.y * 20.0f;
	setTransform(m);
}

Vector2 FlashSpriteInstance::getPosition() const
{
	const Matrix33& m = getTransform();
	return Vector2(m.e13 / 20.0f, m.e23 / 20.0f);
}

void FlashSpriteInstance::setX(float x)
{
	Matrix33 m = getTransform();
	m.e13 = x * 20.0f;
	setTransform(m);
}

float FlashSpriteInstance::getX() const
{
	const Matrix33& m = getTransform();
	return m.e13 / 20.0f;
}

void FlashSpriteInstance::setY(float y)
{
	Matrix33 m = getTransform();
	m.e23 = y * 20.0f;
	setTransform(m);
}

float FlashSpriteInstance::getY() const
{
	const Matrix33& m = getTransform();
	return m.e23 / 20.0f;
}

void FlashSpriteInstance::setSize(const Vector2& size)
{
	Aabb2 bounds = getLocalBounds();
	float extent = (bounds.mx.x - bounds.mn.x) / 20.0f;
	if (abs(extent) > FUZZY_EPSILON)
	{
		Vector2 T, S;
		float R;

		getTransform().decompose(&T, &S, &R);
		S.x = size.x / extent;
		S.y = size.y / extent;
		setTransform(Matrix33::compose(T, S, R));
	}
}

Vector2 FlashSpriteInstance::getSize() const
{
	Aabb2 bounds = getBounds();
	return Vector2(
		(bounds.mx.x - bounds.mn.x) / 20.0f,
		(bounds.mx.y - bounds.mn.y) / 20.0f
	);
}

void FlashSpriteInstance::setWidth(float width)
{
	Aabb2 bounds = getLocalBounds();
	float extent = (bounds.mx.x - bounds.mn.x) / 20.0f;
	if (abs(extent) > FUZZY_EPSILON)
	{
		Vector2 T, S;
		float R;

		getTransform().decompose(&T, &S, &R);
		S.x = width / extent;
		setTransform(Matrix33::compose(T, S, R));
	}
}

float FlashSpriteInstance::getWidth() const
{
	Aabb2 bounds = getBounds();
	return (bounds.mx.x - bounds.mn.x) / 20.0f;
}

void FlashSpriteInstance::setHeight(float height)
{
	Aabb2 bounds = getLocalBounds();
	float extent = (bounds.mx.y - bounds.mn.y) / 20.0f;
	if (abs(extent) > FUZZY_EPSILON)
	{
		Vector2 T, S;
		float R;

		getTransform().decompose(&T, &S, &R);
		S.y = height / extent;
		setTransform(Matrix33::compose(T, S, R));
	}
}

float FlashSpriteInstance::getHeight() const
{
	Aabb2 bounds = getBounds();
	return (bounds.mx.y - bounds.mn.y) / 20.0f;
}

void FlashSpriteInstance::setRotation(float rotation)
{
	Vector2 T, S;
	float R;

	getTransform().decompose(&T, &S, &R);
	R = deg2rad(rotation);
	setTransform(Matrix33::compose(T, S, R));
}

float FlashSpriteInstance::getRotation() const
{
	float R;
	getTransform().decompose(0, 0, &R);
	return rad2deg(R);
}

void FlashSpriteInstance::setScale(const Vector2& scale)
{
	Vector2 T, S;
	float R;

	getTransform().decompose(&T, &S, &R);
	S.x = scale.x / 100.0f;
	S.y = scale.y / 100.0f;
	setTransform(Matrix33::compose(T, S, R));
}

Vector2 FlashSpriteInstance::getScale() const
{
	Vector2 S;
	getTransform().decompose(0, &S, 0);
	return Vector2(
		S.x * 100.0f,
		S.y * 100.0f
	);
}

void FlashSpriteInstance::setXScale(float xscale)
{
	Vector2 T, S;
	float R;

	getTransform().decompose(&T, &S, &R);
	S.x = xscale / 100.0f;
	setTransform(Matrix33::compose(T, S, R));
}

float FlashSpriteInstance::getXScale() const
{
	Vector2 S;
	getTransform().decompose(0, &S, 0);
	return S.x * 100.0f;
}

void FlashSpriteInstance::setYScale(float yscale)
{
	Vector2 T, S;
	float R;

	getTransform().decompose(&T, &S, &R);
	S.y = yscale / 100.0f;
	setTransform(Matrix33::compose(T, S, R));
}

float FlashSpriteInstance::getYScale() const
{
	Vector2 S;
	getTransform().decompose(0, &S, 0);
	return S.y * 100.0f;
}

void FlashSpriteInstance::trace(visitor_t visitor) const
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
	m_canvas = 0;
	m_displayList.reset();

	FlashCharacterInstance::dereference();
}

	}
}
