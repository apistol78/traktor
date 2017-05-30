/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ButtonInstance_H
#define traktor_flash_ButtonInstance_H

#include <map>
#include "Flash/CharacterInstance.h"

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

class Button;
class Movie;

/*! \brief Flash button instance.
 * \ingroup Flash
 */
class T_DLLCLASS ButtonInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	ButtonInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const Button* button);

	virtual ~ButtonInstance();

	virtual void destroy() T_OVERRIDE;

	/*! \brief Get button.
	 *
	 * \return Pointer to button.
	 */
	const Button* getButton() const;

	/*! \brief Get button state.
	 *
	 * \return Button state, combination of Button::StateMasks.
	 */
	uint8_t getState() const;

	/*! \brief Get button local bounds.
	 *
	 * \return Button bounds in local space.
	 */
	Aabb2 getLocalBounds() const;

	/*! \brief Get button character.
	 *
	 * \param referenceId Character identity.
	 * \return Character instance.
	 */
	CharacterInstance* getCharacterInstance(uint16_t referenceId) const;

	virtual void eventMouseDown(int x, int y, int button) T_OVERRIDE T_FINAL;

	virtual void eventMouseUp(int x, int y, int button) T_OVERRIDE T_FINAL;

	virtual void eventMouseMove(int x, int y, int button) T_OVERRIDE T_FINAL;

	virtual Aabb2 getBounds() const T_OVERRIDE T_FINAL;

protected:
	virtual void trace(visitor_t visitor) const T_OVERRIDE T_FINAL;

	virtual void dereference() T_OVERRIDE T_FINAL;

private:
	Ref< const Button > m_button;
	std::map< uint16_t, Ref< CharacterInstance > > m_characterInstances;
	uint8_t m_state;
	bool m_inside;
	bool m_pushed;

	void executeCondition(uint32_t conditionMask);

	void executeScriptEvent(const std::string& eventName);
};

	}
}

#endif	// traktor_flash_ButtonInstance_H
