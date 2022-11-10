/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Misc/SafeDestroy.h"
#include "Spark/Button.h"
#include "Spark/ButtonInstance.h"
#include "Spark/Context.h"
#include "Spark/Dictionary.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ButtonInstance", ButtonInstance, CharacterInstance)

ButtonInstance::ButtonInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const Button* button)
:	CharacterInstance(context, dictionary, parent)
,	m_button(button)
,	m_state(Button::SmUp)
,	m_inside(false)
,	m_pushed(false)
{
	// Create character instances of each character in button's layer list.
	for (const auto& layer : m_button->getButtonLayers())
	{
		Ref< const Character > character = dictionary->getCharacter(layer.characterId);
		if (!character)
			continue;

		Ref< CharacterInstance > characterInstance = character->createInstance(context, dictionary, this, "", Matrix33::identity());
		T_ASSERT(characterInstance);

		m_characterInstances[layer.characterId] = characterInstance;
	}
}

ButtonInstance::~ButtonInstance()
{
	destroy();
}

void ButtonInstance::destroy()
{
	for (auto& it : m_characterInstances)
		safeDestroy(it.second);

	m_characterInstances.clear();

	m_eventPress.removeAll();
	m_eventRelease.removeAll();
	m_eventReleaseOutside.removeAll();
	m_eventRollOver.removeAll();
	m_eventRollOut.removeAll();

	CharacterInstance::destroy();
}

const Button* ButtonInstance::getButton() const
{
	return m_button;
}

uint8_t ButtonInstance::getState() const
{
	return m_state;
}

Aabb2 ButtonInstance::getLocalBounds() const
{
	Aabb2 bounds;

	for (const auto& layer : m_button->getButtonLayers())
	{
		Ref< CharacterInstance > characterInstance = getCharacterInstance(layer.characterId);
		if (!characterInstance)
			continue;

		bounds.contain(characterInstance->getBounds());
	}

	return bounds;
}

CharacterInstance* ButtonInstance::getCharacterInstance(uint16_t referenceId) const
{
	auto it = m_characterInstances.find(referenceId);
	return it != m_characterInstances.end() ? it->second.ptr() : nullptr;
}

void ButtonInstance::eventMouseDown(int x, int y, int button)
{
	if (m_inside && !m_pushed)
	{
		m_eventPress.issue();
		m_state = Button::SmDown;
		m_pushed = true;
	}
}

void ButtonInstance::eventMouseUp(int x, int y, int button)
{
	if (m_inside && m_pushed)
	{
		m_eventRelease.issue();
		m_state = Button::SmOver;
		m_pushed = false;
	}
	else if (!m_inside && m_pushed)
	{
		m_eventReleaseOutside.issue();
		m_state = Button::SmUp;
		m_pushed = false;
	}
}

void ButtonInstance::eventMouseMove(int x, int y, int button)
{
	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));

	// Roll over and out event handling.
	Aabb2 bounds = getLocalBounds();
	bool inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
	if (inside != m_inside)
	{
		if (inside)
		{
			if (button == 0)
			{
				m_eventRollOver.issue();
				m_state = Button::SmOver;
			}
			else
				m_state = Button::SmDown;
		}
		else
		{
			if (button == 0)
			{
				m_eventRollOut.issue();
				m_state = Button::SmUp;
			}
			else
				m_state = Button::SmOver;
		}
		m_inside = inside;
	}
}

Aabb2 ButtonInstance::getBounds() const
{
	return getTransform() * getLocalBounds();
}

	}
}
