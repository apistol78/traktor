#ifndef traktor_flash_FlashSpriteInstance_H
#define traktor_flash_FlashSpriteInstance_H

#include "Flash/FlashCharacterInstance.h"
#include "Flash/FlashDisplayList.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class FlashSprite;
class FlashCharacter;
class ActionVM;
class ActionContext;

/*! \brief Flash sprite instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashSpriteInstance : public FlashCharacterInstance
{
	T_RTTI_CLASS(FlashSpriteInstance)

public:
	FlashSpriteInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashSprite* sprite);

	const FlashSprite* getSprite() const;

	void gotoFrame(uint32_t frameId);

	void gotoPrevious();

	void gotoNext();

	uint32_t getCurrentFrame() const;

	void setPlaying(bool playing);

	bool getPlaying() const;

	void updateDisplayList();

	void preDispatchEvents(ActionVM* actionVM);

	void postDispatchEvents(ActionVM* actionVM);

	FlashDisplayList& getDisplayList();

	void setVisible(bool visible);

	bool isVisible() const;

	FlashSpriteInstance* clone() const;

	virtual bool getMember(const std::wstring& memberName, ActionValue& outMemberValue) const;

	virtual void eventInit(ActionVM* actionVM);

	virtual void eventLoad(ActionVM* actionVM);

	virtual void eventFrame(ActionVM* actionVM);

	virtual void eventKeyDown(ActionVM* actionVM, int keyCode);

	virtual void eventKeyUp(ActionVM* actionVM, int keyCode);

	virtual void eventMouseDown(ActionVM* actionVM, int x, int y, int button);

	virtual void eventMouseUp(ActionVM* actionVM, int x, int y, int button);

	virtual void eventMouseMove(ActionVM* actionVM, int x, int y, int button);

	virtual SwfRect getBounds() const;

	inline int getMouseX() const { return m_mouseX; }

	inline int getMouseY() const { return m_mouseY; }

private:
	Ref< const FlashSprite > m_sprite;
	FlashDisplayList m_displayList;
	uint32_t m_currentFrame;
	uint32_t m_nextFrame;
	uint32_t m_lastUpdateFrame;
	uint32_t m_lastExecutedFrame;
	uint32_t m_skipEnterFrame;
	bool m_initialized;
	bool m_playing;
	bool m_visible;
	bool m_inDispatch;
	bool m_gotoIssued;
	int m_mouseX;
	int m_mouseY;

	void executeScriptEvent(ActionVM* actionVM, const std::wstring& eventName);
};

	}
}

#endif	// traktor_flash_FlashSpriteInstance_H
