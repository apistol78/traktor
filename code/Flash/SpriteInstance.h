/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_SpriteInstance_H
#define traktor_flash_SpriteInstance_H

#include "Flash/CharacterInstance.h"
#include "Flash/DisplayList.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionContext;
class Bitmap;
class Canvas;
class Character;
class EditInstance;
class ISoundRenderer;
class ShapeInstance;
class Sprite;

/*! \brief Flash sprite instance.
 * \ingroup Flash
 */
class T_DLLCLASS SpriteInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	SpriteInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const Sprite* sprite);

	virtual ~SpriteInstance();

	virtual void destroy() T_OVERRIDE;

	const Sprite* getSprite() const { return m_sprite; }

	void setCacheAsBitmap(bool cacheAsBitmap);

	bool getCacheAsBitmap() const { return m_cacheAsBitmap; }

	void gotoFrame(uint32_t frameId);

	void gotoPrevious();

	void gotoNext();

	uint32_t getCurrentFrame() const { return m_currentFrame; }

	void setPlaying(bool playing, bool recursive);

	bool getPlaying() const { return m_playing; }

	/*! \brief Update display list and begin event dispatching. */
	void updateDisplayList();

	/*! \brief Update display list, sounds and begin event dispatching. */
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

	Canvas* createCanvas();

	Canvas* getCanvas() { return m_canvas; }

	int32_t getMouseX() const { return m_mouseX; }

	int32_t getMouseY() const { return m_mouseY; }

	virtual void clearCacheObject() T_OVERRIDE;

	virtual bool enumerateMembers(AlignedVector< uint32_t >& outMemberNames) const T_OVERRIDE T_FINAL;

	virtual bool getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue) T_OVERRIDE T_FINAL;

	virtual void eventInit() T_OVERRIDE T_FINAL;

	virtual void eventConstruct() T_OVERRIDE T_FINAL;

	virtual void eventLoad() T_OVERRIDE T_FINAL;

	virtual void eventFrame() T_OVERRIDE T_FINAL;

	virtual void eventKey(wchar_t unicode) T_OVERRIDE T_FINAL;

	virtual void eventKeyDown(int32_t keyCode) T_OVERRIDE T_FINAL;

	virtual void eventKeyUp(int32_t keyCode) T_OVERRIDE T_FINAL;

	virtual void eventMouseDown(int32_t x, int32_t y, int32_t button) T_OVERRIDE T_FINAL;

	virtual void eventMouseUp(int32_t x, int32_t y, int32_t button) T_OVERRIDE T_FINAL;

	virtual void eventMouseMove(int32_t x, int32_t y, int32_t button) T_OVERRIDE T_FINAL;

	virtual Aabb2 getBounds() const T_OVERRIDE T_FINAL;

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

protected:
	virtual void trace(visitor_t visitor) const T_OVERRIDE T_FINAL;

	virtual void dereference() T_OVERRIDE T_FINAL;

private:
	Ref< const Sprite > m_sprite;
	DisplayList m_displayList;
	Ref< SpriteInstance > m_mask;
	Ref< Canvas > m_canvas;
	int32_t m_mouseX;
	int32_t m_mouseY;
	uint16_t m_currentFrame;
	uint16_t m_lastUpdateFrame;
	uint16_t m_lastExecutedFrame;
	uint16_t m_lastSoundFrame;
	bool m_cacheAsBitmap;
	bool m_initialized;
	bool m_playing;
	bool m_inside;
	bool m_inDispatch;
	bool m_gotoIssued;

	void preDispatchEvents();
};

	}
}

#endif	// traktor_flash_SpriteInstance_H
