/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Timer/Profiler.h"
#include "Spark/Bitmap.h"
#include "Spark/Canvas.h"
#include "Spark/Context.h"
#include "Spark/Dictionary.h"
#include "Spark/Edit.h"
#include "Spark/EditInstance.h"
#include "Spark/ISoundRenderer.h"
#include "Spark/Frame.h"
#include "Spark/Shape.h"
#include "Spark/ShapeInstance.h"
#include "Spark/Sound.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SpriteInstance", SpriteInstance, CharacterInstance)

SpriteInstance::SpriteInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const Sprite* sprite)
:	CharacterInstance(context, dictionary, parent)
,	m_sprite(sprite)
,	m_displayList(context)
,	m_mask(nullptr)
,	m_mouseX(0)
,	m_mouseY(0)
,	m_currentFrame(0)
,	m_lastUpdateFrame(~0)
,	m_lastSoundFrame(~0)
,	m_cacheAsBitmap(false)
,	m_playing(true)
,	m_inDispatch(false)
,	m_gotoIssued(false)
{
	T_ASSERT(m_sprite->getFrameCount() > 0);
}

SpriteInstance::~SpriteInstance()
{
	destroy();
}

void SpriteInstance::destroy()
{
	if (getContext() != nullptr && getContext()->getMovieClip() == this)
		getContext()->setMovieClip(nullptr);

	m_sprite = nullptr;
	m_mask = nullptr;
	m_canvas = nullptr;
	m_playing = false;

	for (const auto& it : m_displayList.getLayers())
	{
		if (it.second.instance)
			it.second.instance->destroy();
	}
	m_displayList.reset();

	setCacheObject(nullptr);
	setParent(nullptr);

	m_eventEnterFrame.removeAll();
	m_eventKeyDown.removeAll();
	m_eventKeyUp.removeAll();
	m_eventMouseDown.removeAll();
	m_eventMouseUp.removeAll();
	m_eventMouseMove.removeAll();
	m_eventPress.removeAll();
	m_eventRelease.removeAll();
	m_eventRollOver.removeAll();
	m_eventRollOut.removeAll();

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

void SpriteInstance::gotoAndPlay(uint32_t frame)
{
	setPlaying(true, false);
	gotoFrame(frame);
}

void SpriteInstance::gotoAndStop(uint32_t frame)
{
	setPlaying(false, false);
	gotoFrame(frame);
}

bool SpriteInstance::gotoAndPlay(const std::string& frameLabel)
{
	int32_t frame = m_sprite->findFrame(frameLabel);
	if (frame < 0)
		return false;

	setPlaying(true, false);
	gotoFrame(frame);
	return true;
}

bool SpriteInstance::gotoAndStop(const std::string& frameLabel)
{
	int32_t frame = m_sprite->findFrame(frameLabel);
	if (frame < 0)
		return false;

	setPlaying(false, false);
	gotoFrame(frame);
	return true;
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

void SpriteInstance::updateDisplayListAndSounds(ISoundRenderer* soundRenderer)
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
					soundRenderer->play(sound);
			}
		}
	}
	m_lastSoundFrame = m_currentFrame;

	m_displayList.forEachVisibleObjectDirect([&] (CharacterInstance* instance) {
		if (&type_of(instance) == &type_of< SpriteInstance >())
			static_cast< SpriteInstance* >(instance)->updateDisplayListAndSounds(soundRenderer);
	});

	preDispatchEvents();
}

Ref< SpriteInstance > SpriteInstance::createEmptyMovieClip(const std::string& clipName, int32_t depth)
{
	Context* context = getContext();
	T_ASSERT(context);

	// Create empty movie character with a single frame.
	Ref< Sprite > emptyClip = new Sprite(0);
	emptyClip->addFrame(new Frame());

	// Create new instance of movie clip.
	Ref< SpriteInstance > emptyClipInstance = checked_type_cast< SpriteInstance* >(emptyClip->createInstance(
		context,
		getDictionary(),
		this,
		clipName,
		Matrix33::identity()
	));
	emptyClipInstance->setName(clipName);

	// Add new instance to display list.
	m_displayList.showObject(depth, emptyClipInstance, true);
	return emptyClipInstance;
}

Ref< EditInstance > SpriteInstance::createTextField(const std::string& textName, int32_t depth, float x, float y, float width, float height)
{
	Context* context = getContext();
	T_ASSERT(context);

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
		Matrix33::identity()
	));

	// Place character at given location.
	editInstance->setTransform(translate(x, y));

	// Show edit character instance.
	getDisplayList().showObject(
		depth,
		editInstance,
		true
	);

	return editInstance;
}

void SpriteInstance::removeMovieClip()
{
	if (!getParent())
		return;

	Ref< SpriteInstance > parentClipInstance = checked_type_cast< SpriteInstance*, false >(getParent());

	DisplayList& parentDisplayList = parentClipInstance->getDisplayList();
	parentDisplayList.removeObject(this);

	if (parentClipInstance->m_mask == this)
		parentClipInstance->m_mask = nullptr;

	destroy();
}

Ref< SpriteInstance > SpriteInstance::clone() const
{
	Ref< SpriteInstance > cloneInstance = checked_type_cast< SpriteInstance* >(m_sprite->createInstance(
		getContext(),
		getDictionary(),
		getParent(),
		"",
		getTransform()
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
	Ref< SpriteInstance > cloneInstance = checked_type_cast< SpriteInstance* >(m_sprite->createInstance(
		getContext(),
		getDictionary(),
		intoParent,
		cloneName,
		getTransform()
	));

	intoParent->getDisplayList().showObject(
		depth,
		cloneInstance,
		true
	);

	return cloneInstance;
}

Ref< ShapeInstance > SpriteInstance::attachBitmap(Bitmap* bm, int32_t depth)
{
	Context* context = getContext();

	// Get dictionary.
	Dictionary* dictionary = getDictionary();
	if (!dictionary)
		return nullptr;

	// Define bitmap symbol.
	uint16_t bitmapId = dictionary->addBitmap(bm);

	// Create a quad shape.
	Ref< Shape > shape = new Shape();
	shape->create(
		bitmapId,
		bm->getWidth() * 20,
		bm->getHeight() * 20
	);

	// Create new instance of shape.
	Ref< ShapeInstance > attachShapeInstance = checked_type_cast< ShapeInstance* >(shape->createInstance(
		context,
		dictionary,
		this,
		"",
		Matrix33::identity()
	));
	T_ASSERT(attachShapeInstance);

	// Add new instance to display list.
	getDisplayList().showObject(depth, attachShapeInstance, true);
	return attachShapeInstance;
}

Aabb2 SpriteInstance::getLocalBounds() const
{
	Aabb2 bounds;

	if (m_canvas)
		bounds = m_canvas->getBounds();

	for (const auto& it : m_displayList.getLayers())
	{
		T_ASSERT(it.second.instance);
		bounds.contain(it.second.instance->getBounds());
	}
	return bounds;
}

Aabb2 SpriteInstance::getVisibleLocalBounds() const
{
	Aabb2 bounds;

	if (m_canvas)
		bounds = m_canvas->getBounds();

	for (const auto& it : m_displayList.getLayers())
	{
		T_ASSERT(it.second.instance);
		if (it.second.instance->isVisible())
			bounds.contain(it.second.instance->getBounds());
	}
	return bounds;
}

void SpriteInstance::setMask(SpriteInstance* mask)
{
	clearCacheObject();
	if ((m_mask = mask) != nullptr)
		m_mask->setVisible(false);
}

CharacterInstance* SpriteInstance::getMember(const std::string& childName) const
{
	for (const auto& it : m_displayList.getLayers())
	{
		if (it.second.instance != nullptr && it.second.instance->getName() == childName)
			return it.second.instance;
	}
	return nullptr;
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

void SpriteInstance::eventFrame()
{
	Context* context = getContext();
	if (!context)
		return;

	Ref< SpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue script assigned event.
	m_eventEnterFrame.issue();

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
	Context* context = getContext();
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
		m_eventKeyDown.issue();

	CharacterInstance::eventKeyDown(keyCode);

	context->setMovieClip(current);
}

void SpriteInstance::eventKeyUp(int32_t keyCode)
{
	Context* context = getContext();
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
		m_eventKeyUp.issue();

	CharacterInstance::eventKeyUp(keyCode);

	context->setMovieClip(current);
}

void SpriteInstance::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
	Context* context = getContext();
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
	m_eventMouseDown.issue();
	
	// Check if we're inside then issue press events.
	if (!context->getPressed() && isEnabled())
	{
		Aabb2 bounds = getVisibleLocalBounds();
		if (bounds.inside(xy))
		{
			if (!m_eventPress.empty() || !m_eventRelease.empty())
			{
				m_eventPress.issue();
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
	Context* context = getContext();
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
	m_eventMouseUp.issue();

	// Check if we're inside then issue press events.
	if (context->getPressed() == this && isEnabled())
	{
		Aabb2 bounds = getVisibleLocalBounds();
		bool inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
		if (inside)
			m_eventRelease.issue();
	}

	CharacterInstance::eventMouseUp(x, y, button);

	context->setMovieClip(current);

	// Finally if mouse up has been issued and we're at the bottom
	// of event chain, we drop reference to pressed character.
	if (!getParent())
		context->setPressed(nullptr);
}

void SpriteInstance::eventMouseMove(int32_t x, int32_t y, int32_t button)
{
	Context* context = getContext();
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
	m_eventMouseMove.issue();

	// Roll over and out event handling.
	if (!context->getRolledOver())
	{
		if (!m_eventRollOver.empty() || !m_eventRollOut.empty())
		{
			Aabb2 bounds = getVisibleLocalBounds();
			bool inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
			if (inside)
			{
				m_eventRollOver.issue();
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

void SpriteInstance::preDispatchEvents()
{
	if (m_inDispatch)
		return;

	m_inDispatch = true;
	m_gotoIssued = false;
}

}
