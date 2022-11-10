/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Spark/CharacterInstance.h"

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

class Button;
class Movie;

/*! Flash button instance.
 * \ingroup Spark
 */
class T_DLLCLASS ButtonInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	explicit ButtonInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const Button* button);

	virtual ~ButtonInstance();

	virtual void destroy() override;

	/*! Get button.
	 *
	 * \return Pointer to button.
	 */
	const Button* getButton() const;

	/*! Get button state.
	 *
	 * \return Button state, combination of Button::StateMasks.
	 */
	uint8_t getState() const;

	/*! Get button local bounds.
	 *
	 * \return Button bounds in local space.
	 */
	Aabb2 getLocalBounds() const;

	/*! Get button character.
	 *
	 * \param referenceId Character identity.
	 * \return Character instance.
	 */
	CharacterInstance* getCharacterInstance(uint16_t referenceId) const;

	virtual void eventMouseDown(int x, int y, int button) override final;

	virtual void eventMouseUp(int x, int y, int button) override final;

	virtual void eventMouseMove(int x, int y, int button) override final;

	virtual Aabb2 getBounds() const override final;

	/*! \group Events */
	//@{

	Event* getEventPress() { return &m_eventPress; }

	Event* getEventRelease() { return &m_eventRelease; }

	Event* getEventReleaseOutside() { return &m_eventReleaseOutside; }

	Event* getEventRollOver() { return &m_eventRollOver; }

	Event* getEventRollOut() { return &m_eventRollOut; }

	//@}

private:
	Ref< const Button > m_button;
	SmallMap< uint16_t, Ref< CharacterInstance > > m_characterInstances;
	uint8_t m_state;
	bool m_inside;
	bool m_pushed;
	Event m_eventPress;
	Event m_eventRelease;
	Event m_eventReleaseOutside;
	Event m_eventRollOver;
	Event m_eventRollOut;
};

	}
}

