#include <limits>
#include "Core/Misc/SafeDestroy.h"
#include "Flash/FlashButton.h"
#include "Flash/FlashButtonInstance.h"
#include "Flash/FlashDictionary.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashButtonInstance", FlashButtonInstance, FlashCharacterInstance)

FlashButtonInstance::FlashButtonInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashButton* button)
:	FlashCharacterInstance(context, "Button", parent)
,	m_button(button)
,	m_state(FlashButton::SmUp)
,	m_inside(false)
,	m_pushed(false)
{
	// Create character instances of each character in button's layer list.
	const FlashButton::button_layers_t& layers = m_button->getButtonLayers();
	for (FlashButton::button_layers_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		Ref< const FlashCharacter > character = context->getDictionary()->getCharacter(i->characterId);
		if (!character)
			continue;

		Ref< FlashCharacterInstance > characterInstance = character->createInstance(context, this, "", Matrix33::identity(), 0, 0);
		T_ASSERT (characterInstance);

		m_characterInstances[i->characterId] = characterInstance;
	}
}

FlashButtonInstance::~FlashButtonInstance()
{
	destroy();
}

void FlashButtonInstance::destroy()
{
	for (std::map< uint16_t, Ref< FlashCharacterInstance > >::iterator i = m_characterInstances.begin(); i != m_characterInstances.end(); ++i)
		safeDestroy(i->second);

	m_characterInstances.clear();

	FlashCharacterInstance::destroy();
}

const FlashButton* FlashButtonInstance::getButton() const
{
	return m_button;
}

uint8_t FlashButtonInstance::getState() const
{
	return m_state;
}

Aabb2 FlashButtonInstance::getLocalBounds() const
{
	Aabb2 bounds;

	const FlashButton::button_layers_t& layers = m_button->getButtonLayers();
	for (FlashButton::button_layers_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		Ref< FlashCharacterInstance > characterInstance = getCharacterInstance(i->characterId);
		if (!characterInstance)
			continue;

		bounds.contain(characterInstance->getBounds());
	}

	return bounds;
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
		executeScriptEvent("onPress");
		m_state = FlashButton::SmDown;
		m_pushed = true;
	}
}

void FlashButtonInstance::eventMouseUp(int x, int y, int button)
{
	if (m_inside && m_pushed)
	{
		executeCondition(FlashButton::CmOverDownToOverUp);
		executeScriptEvent("onRelease");
		m_state = FlashButton::SmOver;
		m_pushed = false;
	}
	else if (!m_inside && m_pushed)
	{
		executeCondition(FlashButton::CmOutDownToIdle);
		executeScriptEvent("onReleaseOutside");
		m_state = FlashButton::SmUp;
		m_pushed = false;
	}
}

void FlashButtonInstance::eventMouseMove0(int x, int y, int button)
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
				executeCondition(FlashButton::CmIdleToOverUp);
				executeScriptEvent("onRollOver");
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
				executeScriptEvent("onRollOut");
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

Aabb2 FlashButtonInstance::getBounds() const
{
	return getTransform() * getLocalBounds();
}

void FlashButtonInstance::trace(visitor_t visitor) const
{
	for (std::map< uint16_t, Ref< FlashCharacterInstance > >::const_iterator i = m_characterInstances.begin(); i != m_characterInstances.end(); ++i)
		visitor(i->second);
	FlashCharacterInstance::trace(visitor);
}

void FlashButtonInstance::dereference()
{
	m_characterInstances.clear();
	FlashCharacterInstance::dereference();
}

void FlashButtonInstance::executeCondition(uint32_t conditionMask)
{
	ActionContext* context = getContext();
	T_ASSERT (context);

	ActionObject* self = getAsObject(context);
	T_ASSERT (self);

	Ref< ActionObject > super = self->getSuper();

	const FlashButton::button_conditions_t& conditions = m_button->getButtonConditions();
	for (FlashButton::button_conditions_t::const_iterator i = conditions.begin(); i != conditions.end(); ++i)
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

void FlashButtonInstance::executeScriptEvent(const std::string& eventName)
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
