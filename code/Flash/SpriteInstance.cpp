/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Timer/Profiler.h"
#include "Flash/Bitmap.h"
#include "Flash/Canvas.h"
#include "Flash/Dictionary.h"
#include "Flash/Edit.h"
#include "Flash/EditInstance.h"
#include "Flash/Frame.h"
#include "Flash/Shape.h"
#include "Flash/ShapeInstance.h"
#include "Flash/Sound.h"
#include "Flash/SoundPlayer.h"
#include "Flash/Sprite.h"
#include "Flash/SpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.SpriteInstance", SpriteInstance, CharacterInstance)

SpriteInstance::SpriteInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const Sprite* sprite)
:	CharacterInstance(context, "MovieClip", dictionary, parent)
,	m_sprite(sprite)
,	m_displayList(context)
,	m_mouseX(0)
,	m_mouseY(0)
,	m_currentFrame(0)
,	m_lastUpdateFrame(~0)
,	m_lastExecutedFrame(~0)
,	m_lastSoundFrame(~0)
,	m_cacheAsBitmap(false)
,	m_initialized(false)
,	m_playing(true)
,	m_inside(false)
,	m_inDispatch(false)
,	m_gotoIssued(false)
{
	T_ASSERT (m_sprite->getFrameCount() > 0);
}

SpriteInstance::~SpriteInstance()
{
	m_sprite = 0;
	m_mask = 0;
	m_canvas = 0;
	m_playing = false;

	const DisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->destroy();
	}
	m_displayList.reset();
}

void SpriteInstance::destroy()
{
	m_sprite = 0;
	m_mask = 0;
	m_canvas = 0;
	m_playing = false;

	const DisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->destroy();
	}
	m_displayList.reset();

	CharacterInstance::destroy();
}

void SpriteInstance::setCacheAsBitmap(bool cacheAsBitmap)
{
	m_cacheAsBitmap = cacheAsBitmap;
}

void SpriteInstance::gotoFrame(uint32_t frameId)
{
	frameId = min(frameId, m_sprite->getFrameCount() - 1);

	if (m_currentFrame > frameId)
	{
		m_displayList.updateBegin(true);
		for (uint32_t i = 0; i <= frameId; ++i)
		{
			Frame* frame = m_sprite->getFrame(i);
			if (frame)
				m_displayList.updateFrame(this, frame);
		}
		m_displayList.updateEnd();
	}
	else if (m_currentFrame < frameId)
	{
		m_displayList.updateBegin(false);
		for (uint32_t i = m_currentFrame + 1; i <= frameId; ++i)
		{
			Frame* frame = m_sprite->getFrame(i);
			if (frame)
				m_displayList.updateFrame(this, frame);
		}
		m_displayList.updateEnd();
	}

	m_lastUpdateFrame =
	m_currentFrame = frameId;
	m_gotoIssued = true;
}

void SpriteInstance::gotoPrevious()
{
	if (m_currentFrame > 0)
		gotoFrame(m_currentFrame - 1);
}

void SpriteInstance::gotoNext()
{
	if (m_currentFrame < m_sprite->getFrameCount() - 1)
		gotoFrame(m_currentFrame + 1);
}

void SpriteInstance::setPlaying(bool playing, bool recursive)
{
	m_playing = playing;
	if (recursive)
	{
		m_displayList.forEachObjectDirect([&] (CharacterInstance* instance) {
			if (&type_of(instance) == &type_of< SpriteInstance >())
				static_cast< SpriteInstance* >(instance)->setPlaying(playing, true);
		});
	}
}

void SpriteInstance::updateDisplayList()
{
	// Update sprite instance's display list.
	if (m_currentFrame < m_lastUpdateFrame)
	{
		m_displayList.updateBegin(true);
		for (uint32_t i = 0; i <= m_currentFrame; ++i)
		{
			Frame* frame = m_sprite->getFrame(i);
			if (frame)
				m_displayList.updateFrame(this, frame);
		}
		m_displayList.updateEnd();
	}
	else if (m_currentFrame > m_lastUpdateFrame)
	{
		m_displayList.updateBegin(false);
		for (uint32_t i = m_lastUpdateFrame + 1; i <= m_currentFrame; ++i)
		{
			Frame* frame = m_sprite->getFrame(i);
			if (frame)
				m_displayList.updateFrame(this, frame);
		}
		m_displayList.updateEnd();
	}
	m_lastUpdateFrame = m_currentFrame;

	m_displayList.forEachVisibleObjectDirect([] (CharacterInstance* instance) {
		if (&type_of(instance) == &type_of< SpriteInstance >())
			static_cast< SpriteInstance* >(instance)->updateDisplayList();
	});

	preDispatchEvents();
}

void SpriteInstance::updateDisplayListAndSounds(SoundPlayer* soundPlayer)
{
	// Update sprite instance's display list.
	if (m_currentFrame < m_lastUpdateFrame)
	{
		m_displayList.updateBegin(true);
		for (uint32_t i = 0; i <= m_currentFrame; ++i)
		{
			Frame* frame = m_sprite->getFrame(i);
			if (frame)
				m_displayList.updateFrame(this, frame);
		}
		m_displayList.updateEnd();
	}
	else if (m_currentFrame > m_lastUpdateFrame)
	{
		m_displayList.updateBegin(false);
		for (uint32_t i = m_lastUpdateFrame + 1; i <= m_currentFrame; ++i)
		{
			Frame* frame = m_sprite->getFrame(i);
			if (frame)
				m_displayList.updateFrame(this, frame);
		}
		m_displayList.updateEnd();
	}
	m_lastUpdateFrame = m_currentFrame;

	// Update sprite instance's sound.
	if (m_lastSoundFrame != m_currentFrame)
	{
		Frame* frame = m_sprite->getFrame(m_currentFrame);
		if (frame)
		{
			const AlignedVector< uint16_t >& startSounds = frame->getStartSounds();
			for (AlignedVector< uint16_t >::const_iterator i = startSounds.begin(); i != startSounds.end(); ++i)
			{
				const Sound* sound = getDictionary()->getSound(*i);
				if (sound)
					soundPlayer->play(sound);
			}
		}
	}
	m_lastSoundFrame = m_currentFrame;

	m_displayList.forEachVisibleObjectDirect([&] (CharacterInstance* instance) {
		if (&type_of(instance) == &type_of< SpriteInstance >())
			static_cast< SpriteInstance* >(instance)->updateDisplayListAndSounds(soundPlayer);
	});

	preDispatchEvents();
}

Ref< SpriteInstance > SpriteInstance::createEmptyMovieClip(const std::string& clipName, int32_t depth)
{
	ActionContext* context = getContext();
	T_ASSERT (context);

	// Create fake character ID.
	uint16_t emptyClipId = depth + 40000;

	// Create empty movie character with a single frame.
	Ref< Sprite > emptyClip = new Sprite(emptyClipId, 0);
	emptyClip->addFrame(new Frame());

	// Create new instance of movie clip.
	Ref< SpriteInstance > emptyClipInstance = checked_type_cast< SpriteInstance* >(emptyClip->createInstance(
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

Ref< EditInstance > SpriteInstance::createTextField(const std::string& textName, int32_t depth, float x, float y, float width, float height)
{
	ActionContext* context = getContext();
	T_ASSERT (context);

	// Get dictionary.
	Dictionary* dictionary = getDictionary();
	if (!dictionary)
		return 0;

	Aabb2 bounds(
		Vector2(0.0f, 0.0f),
		Vector2(width, height)
	);
	Color4f color(0.0f, 0.0f, 0.0f, 0.0f);

	// Create edit character.
	Ref< Edit > edit = new Edit(
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
	Ref< EditInstance > editInstance = checked_type_cast< EditInstance*, false >(edit->createInstance(
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

void SpriteInstance::removeMovieClip()
{
	if (!getParent())
		return;

	ActionContext* context = getContext();
	if (context)
	{
		if (context->getFocus() == this)
			context->setFocus(0);
	}

	Ref< SpriteInstance > parentClipInstance = checked_type_cast< SpriteInstance*, false >(getParent());

	DisplayList& parentDisplayList = parentClipInstance->getDisplayList();
	parentDisplayList.removeObject(this);

	if (parentClipInstance->m_mask == this)
		parentClipInstance->m_mask = 0;

	m_displayList.reset();
	m_mask = 0;
	m_canvas = 0;

	setCacheObject(0);
	setParent(0);
}

Ref< SpriteInstance > SpriteInstance::clone() const
{
	const SmallMap< uint32_t, Ref< const IActionVMImage > >& events = getEvents();
	Ref< SpriteInstance > cloneInstance = checked_type_cast< SpriteInstance* >(m_sprite->createInstance(
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

Ref< SpriteInstance > SpriteInstance::duplicateMovieClip(const std::string& cloneName, int32_t depth)
{
	SpriteInstance* parent = mandatory_non_null_type_cast< SpriteInstance* >(getParent());
	return duplicateMovieClip(cloneName, depth, parent);
}

Ref< SpriteInstance > SpriteInstance::duplicateMovieClip(const std::string& cloneName, int32_t depth, SpriteInstance* intoParent)
{
	const SmallMap< uint32_t, Ref< const IActionVMImage > >& events = getEvents();

	Ref< SpriteInstance > cloneInstance = checked_type_cast< SpriteInstance* >(m_sprite->createInstance(
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

Ref< ShapeInstance > SpriteInstance::attachBitmap(Bitmap* bm, int32_t depth)
{
	ActionContext* context = getContext();

	// Get dictionary.
	Dictionary* dictionary = getDictionary();
	if (!dictionary)
		return 0;

	// Define bitmap symbol.
	uint16_t bitmapId = dictionary->addBitmap(bm);

	// Create a quad shape.
	Ref< Shape > shape = new Shape();
	shape->create(
		bitmapId,
		bm->getWidth() * 20,
		bm->getHeight() * 20
	);

	// Define shape character.
	uint16_t shapeId = dictionary->addCharacter(shape);

	// Create new instance of shape.
	Ref< ShapeInstance > attachShapeInstance = checked_type_cast< ShapeInstance* >(shape->createInstance(context, dictionary, this, "", Matrix33::identity(), 0, 0));
	T_ASSERT (attachShapeInstance);

	// Add new instance to display list.
	getDisplayList().showObject(depth, shapeId, attachShapeInstance, true);
	return attachShapeInstance;
}

Aabb2 SpriteInstance::getLocalBounds() const
{
	Aabb2 bounds;

	if (m_canvas)
		bounds = m_canvas->getBounds();

	const DisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		T_ASSERT (i->second.instance);
		bounds.contain(i->second.instance->getBounds());
	}

	return bounds;
}

Aabb2 SpriteInstance::getVisibleLocalBounds() const
{
	Aabb2 bounds;

	if (m_canvas)
		bounds = m_canvas->getBounds();

	const DisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		T_ASSERT (i->second.instance);
		if (i->second.instance->isVisible())
			bounds.contain(i->second.instance->getBounds());
	}

	return bounds;
}

void SpriteInstance::setMask(SpriteInstance* mask)
{
	clearCacheObject();
	if ((m_mask = mask) != 0)
		m_mask->setVisible(false);
}

Canvas* SpriteInstance::createCanvas()
{
	clearCacheObject();
	if (!m_canvas)
		m_canvas = new Canvas();
	return m_canvas;
}

void SpriteInstance::clearCacheObject()
{
	CharacterInstance::clearCacheObject();
	m_displayList.forEachVisibleObjectDirect([] (CharacterInstance* instance) {
		instance->clearCacheObject();
	});
}

bool SpriteInstance::enumerateMembers(AlignedVector< uint32_t >& outMemberNames) const
{
	// Visible named character in display list.
	const DisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		outMemberNames.push_back(i->second.name);
	return true;
}

bool SpriteInstance::getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue)
{
	// Find visible named character in display list.
	const DisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.name == memberName)
		{
			outMemberValue = ActionValue(i->second.instance->getAsObject(context));
			return true;
		}
	}

	// No character, propagate to base class.
	return CharacterInstance::getMember(context, memberName, outMemberValue);
}

void SpriteInstance::eventInit()
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< SpriteInstance > current = context->getMovieClip();
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

	CharacterInstance::eventInit();

	context->setMovieClip(current);
}

void SpriteInstance::eventConstruct()
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< SpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	CharacterInstance::eventConstruct();

	context->setMovieClip(current);
}

void SpriteInstance::eventLoad()
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< SpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnLoad, ActionValue());

	CharacterInstance::eventLoad();

	context->setMovieClip(current);
}

void SpriteInstance::eventFrame()
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< SpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnEnterFrame, ActionValue());

	// Execute frame scripts.
	if (m_lastExecutedFrame != m_currentFrame)
	{
		m_lastExecutedFrame = m_currentFrame;

		Frame* frame = m_sprite->getFrame(m_currentFrame);
		T_ASSERT (frame);

		const RefArray< const IActionVMImage >& scripts = frame->getActionScripts();
		if (!scripts.empty())
		{
			T_PROFILER_SCOPE(L"Flash frame scripts");

			ActionObject* self = getAsObject(context);
			T_ASSERT (self);

			Ref< ActionObject > super = self->getSuper();
			for (RefArray< const IActionVMImage >::const_iterator i = scripts.begin(); i != scripts.end(); ++i)
			{
				T_ANONYMOUS_VAR(ActionValuePool::Scope)(context->getPool());
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
		}
	}

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObject([] (CharacterInstance* instance) {
		instance->eventFrame();
	});

	CharacterInstance::eventFrame();

	context->setMovieClip(current);

	// Update current frame index.
	if (m_inDispatch)
	{
		if (m_playing && !m_gotoIssued)
			m_currentFrame = (m_currentFrame + 1) % m_sprite->getFrameCount();
		m_inDispatch = false;
	}
}

void SpriteInstance::eventKey(wchar_t unicode)
{
	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObject([&] (CharacterInstance* instance) {
		instance->eventKey(unicode);
	});

	CharacterInstance::eventKey(unicode);
}

void SpriteInstance::eventKeyDown(int32_t keyCode)
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< SpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObject([&] (CharacterInstance* instance) {
		instance->eventKeyDown(keyCode);
	});

	// Issue script assigned event.
	if (context->getFocus() == this)
		executeScriptEvent(ActionContext::IdOnKeyDown, ActionValue());

	CharacterInstance::eventKeyDown(keyCode);

	context->setMovieClip(current);
}

void SpriteInstance::eventKeyUp(int32_t keyCode)
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< SpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObject([&] (CharacterInstance* instance) {
		instance->eventKeyUp(keyCode);
	});

	// Issue script assigned event.
	if (context->getFocus() == this)
		executeScriptEvent(ActionContext::IdOnKeyUp, ActionValue());

	CharacterInstance::eventKeyUp(keyCode);

	context->setMovieClip(current);
}

void SpriteInstance::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< SpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Remove focus item if we're root, focus might be shifted.
	if (!getParent())
		context->setFocus(0);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObjectReverse([&] (CharacterInstance* instance) {
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
	CharacterInstance::eventMouseDown(x, y, button);

	context->setMovieClip(current);
}

void SpriteInstance::eventMouseUp(int32_t x, int32_t y, int32_t button)
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< SpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObjectReverse([&] (CharacterInstance* instance) {
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

	CharacterInstance::eventMouseUp(x, y, button);

	context->setMovieClip(current);
	
	// Finally if mouse up has been issued and we're at the bottom
	// of event chain, we drop reference to pressed character.
	if (!getParent())
		context->setPressed(0);
}

void SpriteInstance::eventMouseMove(int32_t x, int32_t y, int32_t button)
{
	ActionContext* context = getContext();
	if (!context)
		return;

	Ref< SpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Issue events on "visible" characters.
	m_displayList.forEachVisibleObjectReverse([&] (CharacterInstance* instance) {
		instance->eventMouseMove(x, y, button);
	});

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnMouseMove, ActionValue());

	// Roll over and out event handling.
	if (!context->getRolledOver())
	{
		if (haveScriptEvent(ActionContext::IdOnRollOver) || haveScriptEvent(ActionContext::IdOnRollOut))
		{
			Aabb2 bounds = getVisibleLocalBounds();
			bool inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
			if (inside)
			{
				executeScriptEvent(ActionContext::IdOnRollOver, ActionValue());
				context->setRolledOver(this);
			}
		}
	}

	CharacterInstance::eventMouseMove(x, y, button);

	context->setMovieClip(current);
}

Aabb2 SpriteInstance::getBounds() const
{
	return getTransform() * getVisibleLocalBounds();
}

void SpriteInstance::setPosition(const Vector2& position)
{
	Matrix33 m = getTransform();
	m.e13 = position.x * 20.0f;
	m.e23 = position.y * 20.0f;
	setTransform(m);
}

Vector2 SpriteInstance::getPosition() const
{
	const Matrix33& m = getTransform();
	return Vector2(m.e13 / 20.0f, m.e23 / 20.0f);
}

void SpriteInstance::setX(float x)
{
	Matrix33 m = getTransform();
	m.e13 = x * 20.0f;
	setTransform(m);
}

float SpriteInstance::getX() const
{
	const Matrix33& m = getTransform();
	return m.e13 / 20.0f;
}

void SpriteInstance::setY(float y)
{
	Matrix33 m = getTransform();
	m.e23 = y * 20.0f;
	setTransform(m);
}

float SpriteInstance::getY() const
{
	const Matrix33& m = getTransform();
	return m.e23 / 20.0f;
}

void SpriteInstance::setSize(const Vector2& size)
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

Vector2 SpriteInstance::getSize() const
{
	Aabb2 bounds = getBounds();
	return Vector2(
		(bounds.mx.x - bounds.mn.x) / 20.0f,
		(bounds.mx.y - bounds.mn.y) / 20.0f
	);
}

void SpriteInstance::setWidth(float width)
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

float SpriteInstance::getWidth() const
{
	Aabb2 bounds = getBounds();
	return (bounds.mx.x - bounds.mn.x) / 20.0f;
}

void SpriteInstance::setHeight(float height)
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

float SpriteInstance::getHeight() const
{
	Aabb2 bounds = getBounds();
	return (bounds.mx.y - bounds.mn.y) / 20.0f;
}

void SpriteInstance::setRotation(float rotation)
{
	Vector2 T, S;
	float R;

	getTransform().decompose(&T, &S, &R);
	R = deg2rad(rotation);
	setTransform(Matrix33::compose(T, S, R));
}

float SpriteInstance::getRotation() const
{
	float R;
	getTransform().decompose(0, 0, &R);
	return rad2deg(R);
}

void SpriteInstance::setScale(const Vector2& scale)
{
	Vector2 T, S;
	float R;

	getTransform().decompose(&T, &S, &R);
	S.x = scale.x / 100.0f;
	S.y = scale.y / 100.0f;
	setTransform(Matrix33::compose(T, S, R));
}

Vector2 SpriteInstance::getScale() const
{
	Vector2 S;
	getTransform().decompose(0, &S, 0);
	return Vector2(
		S.x * 100.0f,
		S.y * 100.0f
	);
}

void SpriteInstance::setXScale(float xscale)
{
	Vector2 T, S;
	float R;

	getTransform().decompose(&T, &S, &R);
	S.x = xscale / 100.0f;
	setTransform(Matrix33::compose(T, S, R));
}

float SpriteInstance::getXScale() const
{
	Vector2 S;
	getTransform().decompose(0, &S, 0);
	return S.x * 100.0f;
}

void SpriteInstance::setYScale(float yscale)
{
	Vector2 T, S;
	float R;

	getTransform().decompose(&T, &S, &R);
	S.y = yscale / 100.0f;
	setTransform(Matrix33::compose(T, S, R));
}

float SpriteInstance::getYScale() const
{
	Vector2 S;
	getTransform().decompose(0, &S, 0);
	return S.y * 100.0f;
}

void SpriteInstance::trace(visitor_t visitor) const
{
	visitor(m_mask);

	const DisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		visitor(i->second.instance);

	CharacterInstance::trace(visitor);
}

void SpriteInstance::dereference()
{
	m_mask = 0;
	m_canvas = 0;
	m_displayList.reset();

	CharacterInstance::dereference();
}

void SpriteInstance::preDispatchEvents()
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
}

	}
}
