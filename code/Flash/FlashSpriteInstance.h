#ifndef traktor_flash_FlashSpriteInstance_H
#define traktor_flash_FlashSpriteInstance_H

#include "Flash/FlashCharacterInstance.h"
#include "Flash/FlashDisplayList.h"

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
class FlashCanvas;
class FlashCharacter;
class FlashSoundPlayer;
class FlashSprite;

/*! \brief Flash sprite instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashSpriteInstance : public FlashCharacterInstance
{
	T_RTTI_CLASS;

public:
	FlashSpriteInstance(ActionContext* context, FlashDictionary* dictionary, FlashCharacterInstance* parent, const FlashSprite* sprite);

	virtual ~FlashSpriteInstance();

	void destroy();

	const FlashSprite* getSprite() const { return m_sprite; }

	void setCacheAsBitmap(bool cacheAsBitmap);

	bool getCacheAsBitmap() const { return m_cacheAsBitmap; }

	void setOpaqueBackground(bool opaqueBackground);

	bool getOpaqueBackground() const { return m_opaqueBackground; }

	void gotoFrame(uint32_t frameId);

	void gotoPrevious();

	void gotoNext();

	uint32_t getCurrentFrame() const { return m_currentFrame; }

	void setPlaying(bool playing);

	bool getPlaying() const { return m_playing; }

	void updateDisplayList();

	FlashDisplayList& getDisplayList() { return m_displayList; }

	const FlashDisplayList& getDisplayList() const { return m_displayList; }

	void updateSounds(FlashSoundPlayer* soundPlayer);

	Ref< FlashSpriteInstance > createEmptyMovieClip(const std::string& clipName, int32_t depth);

	void removeMovieClip();

	Ref< FlashSpriteInstance > clone() const;

	Aabb2 getLocalBounds() const;

	Aabb2 getVisibleLocalBounds() const;

	void setMask(FlashSpriteInstance* maskInstance);

	FlashSpriteInstance* getMask() { return m_mask; }

	FlashCanvas* createCanvas();

	FlashCanvas* getCanvas() { return m_canvas; }

	int32_t getMouseX() const { return m_mouseX; }

	int32_t getMouseY() const { return m_mouseY; }

	virtual bool enumerateMembers(std::vector< uint32_t >& outMemberNames) const T_OVERRIDE T_FINAL;

	virtual bool getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue) T_OVERRIDE T_FINAL;

	virtual void preDispatchEvents() T_OVERRIDE T_FINAL;

	virtual void postDispatchEvents() T_OVERRIDE T_FINAL;

	virtual void eventInit() T_OVERRIDE T_FINAL;

	virtual void eventConstruct() T_OVERRIDE T_FINAL;

	virtual void eventLoad() T_OVERRIDE T_FINAL;

	virtual void eventFrame() T_OVERRIDE T_FINAL;

	virtual void eventKey(wchar_t unicode) T_OVERRIDE T_FINAL;

	virtual void eventKeyDown(int32_t keyCode) T_OVERRIDE T_FINAL;

	virtual void eventKeyUp(int32_t keyCode) T_OVERRIDE T_FINAL;

	virtual void eventMouseDown(int32_t x, int32_t y, int32_t button) T_OVERRIDE T_FINAL;

	virtual void eventMouseUp(int32_t x, int32_t y, int32_t button) T_OVERRIDE T_FINAL;

	virtual void eventMouseMove0(int32_t x, int32_t y, int32_t button) T_OVERRIDE T_FINAL;

	virtual void eventMouseMove1(int32_t x, int32_t y, int32_t button) T_OVERRIDE T_FINAL;

	virtual Aabb2 getBounds() const T_OVERRIDE T_FINAL;

protected:
	virtual void trace(visitor_t visitor) const T_OVERRIDE T_FINAL;

	virtual void dereference() T_OVERRIDE T_FINAL;

private:
	Ref< const FlashSprite > m_sprite;
	FlashDisplayList m_displayList;
	Ref< FlashSpriteInstance > m_mask;
	Ref< FlashCanvas > m_canvas;
	RefArray< FlashCharacterInstance > m_visibleCharacters;
	uint32_t m_currentFrame;
	uint32_t m_nextFrame;
	uint32_t m_lastUpdateFrame;
	uint32_t m_lastExecutedFrame;
	uint32_t m_lastSoundFrame;
	uint32_t m_skipEnterFrame;
	bool m_cacheAsBitmap;
	bool m_opaqueBackground;
	bool m_initialized;
	bool m_playing;
	bool m_visible;
	bool m_enabled;
	bool m_inside;
	bool m_inDispatch;
	bool m_gotoIssued;
	int32_t m_mouseX;
	int32_t m_mouseY;
	int32_t m_maskCount;
};

	}
}

#endif	// traktor_flash_FlashSpriteInstance_H
