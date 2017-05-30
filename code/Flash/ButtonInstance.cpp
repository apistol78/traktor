/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Misc/SafeDestroy.h"
#include "Flash/Button.h"
#include "Flash/ButtonInstance.h"
#include "Flash/Dictionary.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ButtonInstance", ButtonInstance, CharacterInstance)

ButtonInstance::ButtonInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const Button* button)
:	CharacterInstance(context, "Button", dictionary, parent)
,	m_button(button)
,	m_state(Button::SmUp)
,	m_inside(false)
,	m_pushed(false)
{
	// Create character instances of each character in button's layer list.
	const Button::button_layers_t& layers = m_button->getButtonLayers();
	for (Button::button_layers_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		Ref< const Character > character = dictionary->getCharacter(i->characterId);
		if (!character)
			continue;

		Ref< CharacterInstance > characterInstance = character->createInstance(context, dictionary, this, "", Matrix33::identity(), 0, 0);
		T_ASSERT (characterInstance);

		m_characterInstances[i->characterId] = characterInstance;
	}
}

ButtonInstance::~ButtonInstance()
{
	for (std::map< uint16_t, Ref< CharacterInstance > >::iterator i = m_characterInstances.begin(); i != m_characterInstances.end(); ++i)
		safeDestroy(i->second);

	m_characterInstances.clear();
}

void ButtonInstance::destroy()
{
	for (std::map< uint16_t, Ref< CharacterInstance > >::iterator i = m_characterInstances.begin(); i != m_characterInstances.end(); ++i)
		safeDestroy(i->second);

	m_characterInstances.clear();

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

	const Button::button_layers_t& layers = m_button->getButtonLayers();
	for (Button::button_layers_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		Ref< CharacterInstance > characterInstance = getCharacterInstance(i->characterId);
		if (!characterInstance)
			continue;

		bounds.contain(characterInstance->getBounds());
	}

	return bounds;
}

CharacterInstance* ButtonInstance::getCharacterInstance(uint16_t referenceId) const
{
	std::map< uint16_t, Ref< CharacterInstance > >::const_iterator i = m_characterInstances.find(referenceId);
	return i != m_characterInstances.end() ? i->second.ptr() : 0;
}

void ButtonInstance::eventMouseDown(int x, int y, int button)
{
	if (m_inside && !m_pushed)
	{
		executeCondition(Button::CmOverUpToOverDown);
		executeScriptEvent("onPress");
		m_state = Button::SmDown;
		m_pushed = true;
	}
}

void ButtonInstance::eventMouseUp(int x, int y, int button)
{
	if (m_inside && m_pushed)
	{
		executeCondition(Button::CmOverDownToOverUp);
		executeScriptEvent("onRelease");
		m_state = Button::SmOver;
		m_pushed = false;
	}
	else if (!m_inside && m_pushed)
	{
		executeCondition(Button::CmOutDownToIdle);
		executeScriptEvent("onReleaseOutside");
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
				executeCondition(Button::CmIdleToOverUp);
				executeScriptEvent("onRollOver");
				m_state = Button::SmOver;
			}
			else
			{
				executeCondition(Button::CmOutDownToOverDown);
				m_state = Button::SmDown;
			}
		}
		else
		{
			if (button == 0)
			{
				executeCondition(Button::CmOverUpToIdle);
				executeScriptEvent("onRollOut");
				m_state = Button::SmUp;
			}
			else
			{
				executeCondition(Button::CmOverDownToOutDown);
				m_state = Button::SmOver;
			}
		}
		m_inside = inside;
	}
}

Aabb2 ButtonInstance::getBounds() const
{
	return getTransform() * getLocalBounds();
}

void ButtonInstance::trace(visitor_t visitor) const
{
	for (std::map< uint16_t, Ref< CharacterInstance > >::const_iterator i = m_characterInstances.begin(); i != m_characterInstances.end(); ++i)
		visitor(i->second);
	CharacterInstance::trace(visitor);
}

void ButtonInstance::dereference()
{
	m_characterInstances.clear();
	CharacterInstance::dereference();
}

void ButtonInstance::executeCondition(uint32_t conditionMask)
{
	ActionContext* context = getContext();
	T_ASSERT (context);

	ActionObject* self = getAsObject(context);
	T_ASSERT (self);

	Ref< ActionObject > super = self->getSuper();

	const Button::button_conditions_t& conditions = m_button->getButtonConditions();
	for (Button::button_conditions_t::const_iterator i = conditions.begin(); i != conditions.end(); ++i)
	{
		if ((i->mask & conditionMask) == 0)
			continue;

		ActionFrame callFrame(
			context,
			self,
			4,
			0,
			0
		);

		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		callFrame.setVariable(ActionContext::IdGlobal, ActionValue(context->getGlobal()));

		i->script->execute(&callFrame);
	}
}

void ButtonInstance::executeScriptEvent(const std::string& eventName)
{
	ActionContext* context = getContext();
	T_ASSERT (context);

	ActionObject* self = getAsObject(context);
	T_ASSERT (self);

	ActionValue memberValue;
	if (!self->getMember(eventName, memberValue))
		return;

	Ref< ActionFunction > eventFunction = memberValue.getObject< ActionFunction >();
	if (!eventFunction)
		return;

	eventFunction->call(self);
}

	}
}
