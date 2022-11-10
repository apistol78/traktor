/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spark/CharacterInstance.h"
#include "Spark/DisplayList.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class Bitmap;
class Context;
class Canvas;
class Character;
class EditInstance;
class ISoundRenderer;
class ShapeInstance;
class Sprite;

/*! Sprite instance.
 * \ingroup Spark
 */
class T_DLLCLASS SpriteInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	explicit SpriteInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const Sprite* sprite);

	virtual ~SpriteInstance();

	virtual void destroy() override;

	const Sprite* getSprite() const { return m_sprite; }

	void setCacheAsBitmap(bool cacheAsBitmap);

	bool getCacheAsBitmap() const { return m_cacheAsBitmap; }

	void gotoFrame(uint32_t frameId);

	void gotoPrevious();

	void gotoNext();

	void gotoAndPlay(uint32_t frame);

	void gotoAndStop(uint32_t frame);

	bool gotoAndPlay(const std::string& frameLabel);

	bool gotoAndStop(const std::string& frameLabel);

	uint32_t getCurrentFrame() const { return m_currentFrame; }

	void setPlaying(bool playing, bool recursive);

	bool getPlaying() const { return m_playing; }

	/*! Update display list and begin event dispatching. */
	void updateDisplayList();

	/*! Update display list, sounds and begin event dispatching. */
	void updateDisplayListAndSounds(ISoundRenderer* soundRenderer);

	DisplayList& getDisplayList() { return m_displayList; }

	const DisplayList& getDisplayList() const { return m_displayList; }

	Ref< SpriteInstance > createEmptyMovieClip(const std::string& clipName, int32_t depth);

	Ref< EditInstance > createTextField(const std::string& textName, int32_t depth, float x, float y, float width, float height);

	void removeMovieClip();

	Ref< SpriteInstance > clone() const;

	Ref< SpriteInstance > duplicateMovieClip(const std::string& cloneName, int32_t depth);

	Ref< SpriteInstance > duplicateMovieClip(const std::string& cloneName, int32_t depth, SpriteInstance* intoParent);

	Ref< ShapeInstance > attachBitmap(Bitmap* bm, int32_t depth);

	Aabb2 getLocalBounds() const;

	Aabb2 getVisibleLocalBounds() const;

	void setMask(SpriteInstance* maskInstance);

	SpriteInstance* getMask() { return m_mask; }

	CharacterInstance* getMember(const std::string& childName) const;

	Canvas* createCanvas();

	Canvas* getCanvas() { return m_canvas; }

	int32_t getMouseX() const { return m_mouseX; }

	int32_t getMouseY() const { return m_mouseY; }

	virtual void clearCacheObject() override;

	virtual void eventFrame() override final;

	virtual void eventKey(wchar_t unicode) override final;

	virtual void eventKeyDown(int32_t keyCode) override final;

	virtual void eventKeyUp(int32_t keyCode) override final;

	virtual void eventMouseDown(int32_t x, int32_t y, int32_t button) override final;

	virtual void eventMouseUp(int32_t x, int32_t y, int32_t button) override final;

	virtual void eventMouseMove(int32_t x, int32_t y, int32_t button) override final;

	virtual Aabb2 getBounds() const override final;

	/*! \group Convenience methods, commonly used by scripting. */
	//@{

	void setPosition(const Vector2& position);

	Vector2 getPosition() const;

	void setX(float x);

	float getX() const;

	void setY(float y);

	float getY() const;

	void setSize(const Vector2& size);

	Vector2 getSize() const;

	void setWidth(float width);

	float getWidth() const;

	void setHeight(float height);

	float getHeight() const;

	void setRotation(float rotation);

	float getRotation() const;

	void setScale(const Vector2& scale);

	Vector2 getScale() const;

	void setXScale(float xscale);

	float getXScale() const;

	void setYScale(float yscale);

	float getYScale() const;

	//@}

	/*! \group Events */
	//@{

	Event* getEventEnterFrame() { return &m_eventEnterFrame; }

	Event* getEventKeyDown() { return &m_eventKeyDown; }

	Event* getEventKeyUp() { return &m_eventKeyUp; }

	Event* getEventMouseDown() { return &m_eventMouseDown; }

	Event* getEventMouseUp() { return &m_eventMouseUp; }

	Event* getEventMouseMove() { return &m_eventMouseMove; }

	Event* getEventPress() { return &m_eventPress; }

	Event* getEventRelease() { return &m_eventRelease; }

	Event* getEventRollOver() { return &m_eventRollOver; }

	Event* getEventRollOut() { return &m_eventRollOut; }

	//@}

private:
	Ref< const Sprite > m_sprite;
	DisplayList m_displayList;
	SpriteInstance* m_mask;
	Ref< Canvas > m_canvas;
	int32_t m_mouseX;
	int32_t m_mouseY;
	uint16_t m_currentFrame;
	uint16_t m_lastUpdateFrame;
	uint16_t m_lastSoundFrame;
	bool m_cacheAsBitmap;
	bool m_playing;
	bool m_inDispatch;
	bool m_gotoIssued;
	Event m_eventEnterFrame;
	Event m_eventKeyDown;
	Event m_eventKeyUp;
	Event m_eventMouseDown;
	Event m_eventMouseUp;
	Event m_eventMouseMove;
	Event m_eventPress;
	Event m_eventRelease;
	Event m_eventRollOver;
	Event m_eventRollOut;

	void preDispatchEvents();
};

	}
}

