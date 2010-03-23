#ifndef traktor_flash_FlashButtonInstance_H
#define traktor_flash_FlashButtonInstance_H

#include "Flash/FlashCharacterInstance.h"

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

class FlashButton;
class FlashMovie;

/*! \brief Flash button instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashButtonInstance : public FlashCharacterInstance
{
	T_RTTI_CLASS;

public:
	FlashButtonInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashButton* button);

	/*! \brief Get button.
	 *
	 * \return Pointer to button.
	 */
	const FlashButton* getButton() const;

	/*! \brief Get button state.
	 *
	 * \return Button state, combination of FlashButton::StateMasks.
	 */
	uint8_t getState() const;

	/*! \brief Get button character.
	 *
	 * \param referenceId Character identity.
	 * \return Character instance.
	 */
	FlashCharacterInstance* getCharacterInstance(uint16_t referenceId) const;

	virtual void eventMouseDown(IActionVM* vm, int x, int y, int button);

	virtual void eventMouseUp(IActionVM* actionVM, int x, int y, int button);

	virtual void eventMouseMove(IActionVM* actionVM, int x, int y, int button);

	virtual SwfRect getBounds() const;

private:
	Ref< const FlashButton > m_button;
	std::map< uint16_t, Ref< FlashCharacterInstance > > m_characterInstances;
	uint8_t m_state;
	bool m_inside;
	bool m_pushed;

	void executeCondition(IActionVM* actionVM, uint32_t conditionMask);

	void executeScriptEvent(IActionVM* actionVM, const std::wstring& eventName);
};

	}
}

#endif	// traktor_flash_FlashButtonInstance_H
