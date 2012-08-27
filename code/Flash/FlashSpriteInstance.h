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
class FlashSprite;

/*! \brief Flash sprite instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashSpriteInstance : public FlashCharacterInstance
{
	T_RTTI_CLASS;

public:
	FlashSpriteInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashSprite* sprite);

	virtual void destroy();

	const FlashSprite* getSprite() const;

	void gotoFrame(uint32_t frameId);

	void gotoPrevious();

	void gotoNext();

	uint32_t getCurrentFrame() const;

	void setPlaying(bool playing);

	bool getPlaying() const;

	void updateDisplayList();

	FlashDisplayList& getDisplayList();

	Ref< FlashSpriteInstance > clone() const;

	SwfRect getLocalBounds() const;

	void setMask(FlashSpriteInstance* maskInstance);

	FlashSpriteInstance* getMask();

	FlashCanvas* createCanvas();

	FlashCanvas* getCanvas() { return m_canvas; }

	int32_t getMouseX() const { return m_mouseX; }

	int32_t getMouseY() const { return m_mouseY; }

	virtual bool getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue);

	virtual void preDispatchEvents();

	virtual void postDispatchEvents();

	virtual void eventInit();

	virtual void eventLoad();

	virtual void eventFrame();

	virtual void eventKeyDown(int32_t keyCode);

	virtual void eventKeyUp(int32_t keyCode);

	virtual void eventMouseDown(int32_t x, int32_t y, int32_t button);

	virtual void eventMouseUp(int32_t x, int32_t y, int32_t button);

	virtual void eventMouseMove(int32_t x, int32_t y, int32_t button);

	virtual SwfRect getBounds() const;

protected:
	virtual void trace(const IVisitor& visitor) const;

	virtual void dereference();

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
	uint32_t m_skipEnterFrame;
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
	uint32_t m_idOnLoad;
	uint32_t m_idOnEnterFrame;
	uint32_t m_idOnKeyDown;
	uint32_t m_idOnKeyUp;
	uint32_t m_idOnMouseDown;
	uint32_t m_idOnPress;
	uint32_t m_idOnMouseUp;
	uint32_t m_idOnRelease;
	uint32_t m_idOnMouseMove;
	uint32_t m_idOnRollOver;
	uint32_t m_idOnRollOut;

	void executeScriptEvent(uint32_t eventName);
};

	}
}

#endif	// traktor_flash_FlashSpriteInstance_H
