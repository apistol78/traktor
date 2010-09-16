#include <limits>
#include "Flash/FlashButtonInstance.h"
#include "Flash/FlashButton.h"
#include "Flash/FlashMovie.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionScript.h"
#include "Flash/Action/IActionVM.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashButtonInstance", FlashButtonInstance, FlashCharacterInstance)

FlashButtonInstance::FlashButtonInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashButton* button)
:	FlashCharacterInstance(context, L"Button", parent)
,	m_button(button)
,	m_state(FlashButton::SmUp)
,	m_inside(false)
,	m_pushed(false)
{
	// Create character instances of each character in button's layer list.
	const FlashButton::button_layers_t& layers = m_button->getButtonLayers();
	for (FlashButton::button_layers_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		Ref< const FlashCharacter > character = context->getMovie()->getCharacter(i->characterId);
		if (!character)
			continue;

		Ref< FlashCharacterInstance > characterInstance = character->createInstance(context, this);
		T_ASSERT (characterInstance);

		m_characterInstances[i->characterId] = characterInstance;
	}
}

const FlashButton* FlashButtonInstance::getButton() const
{
	return m_button;
}

uint8_t FlashButtonInstance::getState() const
{
	return m_state;
}

FlashCharacterInstance* FlashButtonInstance::getCharacterInstance(uint16_t referenceId) const
{
	std::map< uint16_t, Ref< FlashCharacterInstance > >::const_iterator i = m_characterInstances.find(referenceId);
	return i != m_characterInstances.end() ? i->second.ptr() : 0;
}

void FlashButtonInstance::eventMouseDown(int x, int y, int button)
{
	if (m_inside && !m_pushed)
	{
		executeCondition(FlashButton::CmOverUpToOverDown);
		executeScriptEvent(L"onPress");
		m_state = FlashButton::SmDown;
		m_pushed = true;
	}
}

void FlashButtonInstance::eventMouseUp(int x, int y, int button)
{
	if (m_inside && m_pushed)
	{
		executeCondition(FlashButton::CmOverDownToOverUp);
		executeScriptEvent(L"onRelease");
		m_state = FlashButton::SmOver;
		m_pushed = false;
	}
	else if (!m_inside && m_pushed)
	{
		executeCondition(FlashButton::CmOutDownToIdle);
		executeScriptEvent(L"onReleaseOutside");
		m_state = FlashButton::SmUp;
		m_pushed = false;
	}
}

void FlashButtonInstance::eventMouseMove(int x, int y, int button)
{
	SwfRect bounds = getBounds();
	bool inside = (x >= bounds.min.x && y >= bounds.min.y && x <= bounds.max.x && y <= bounds.max.y);
	if (inside != m_inside)
	{
		if (inside)
		{
			if (button == 0)
			{
				executeCondition(FlashButton::CmIdleToOverUp);
				executeScriptEvent(L"onRollOver");
				m_state = FlashButton::SmOver;
			}
			else
			{
				executeCondition(FlashButton::CmOutDownToOverDown);
				m_state = FlashButton::SmDown;
			}
		}
		else
		{
			if (button == 0)
			{
				executeCondition(FlashButton::CmOverUpToIdle);
				executeScriptEvent(L"onRollOut");
				m_state = FlashButton::SmUp;
			}
			else
			{
				executeCondition(FlashButton::CmOverDownToOutDown);
				m_state = FlashButton::SmOver;
			}
		}
		m_inside = inside;
	}
}

SwfRect FlashButtonInstance::getBounds() const
{
	SwfRect bounds =
	{
		Vector2( std::numeric_limits< float >::max(),  std::numeric_limits< float >::max()),
		Vector2(-std::numeric_limits< float >::max(), -std::numeric_limits< float >::max())
	};

	const FlashButton::button_layers_t& layers = m_button->getButtonLayers();
	for (FlashButton::button_layers_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		Ref< FlashCharacterInstance > characterInstance = getCharacterInstance(i->characterId);
		if (!characterInstance)
			continue;

		SwfRect characterBounds = characterInstance->getBounds();
		bounds.min.x = std::min(bounds.min.x, characterBounds.min.x);
		bounds.min.y = std::min(bounds.min.y, characterBounds.min.y);
		bounds.max.x = std::max(bounds.max.x, characterBounds.max.x);
		bounds.max.y = std::max(bounds.max.y, characterBounds.max.y);
	}

	bounds.min = getTransform() * bounds.min;
	bounds.max = getTransform() * bounds.max;

	return bounds;
}

void FlashButtonInstance::trace(const IVisitor& visitor) const
{
	//visitor(m_button);
	for (std::map< uint16_t, Ref< FlashCharacterInstance > >::const_iterator i = m_characterInstances.begin(); i != m_characterInstances.end(); ++i)
		visitor(i->second);
	FlashCharacterInstance::trace(visitor);
}

void FlashButtonInstance::dereference()
{
	//m_button = 0;
	m_characterInstances.clear();
	FlashCharacterInstance::dereference();
}

void FlashButtonInstance::executeCondition(uint32_t conditionMask)
{
	const FlashButton::button_conditions_t& conditions = m_button->getButtonConditions();
	for (FlashButton::button_conditions_t::const_iterator i = conditions.begin(); i != conditions.end(); ++i)
	{
		if ((i->mask & conditionMask) == 0)
			continue;

		ActionContext* context = getContext();
		ActionFrame callFrame(context, this, i->script->getCode(), i->script->getCodeSize(), 4, 0, 0);
		context->getVM()->execute(&callFrame);
	}
}

void FlashButtonInstance::executeScriptEvent(const std::wstring& eventName)
{
	ActionValue memberValue;
	if (!getLocalMember(eventName, memberValue))
		return;

	Ref< ActionFunction > eventFunction = memberValue.getObject< ActionFunction >();
	if (!eventFunction)
		return;

	ActionFrame callFrame(getContext(), this, 0, 0, 4, 0, 0);
	eventFunction->call(&callFrame, this);
}

	}
}
