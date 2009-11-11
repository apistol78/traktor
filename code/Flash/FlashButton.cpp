#include "Flash/FlashButton.h"
#include "Flash/FlashButtonInstance.h"
#include "Core/Heap/GcNew.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashButton", FlashButton, FlashCharacter)

FlashButton::FlashButton(uint16_t id)
:	FlashCharacter(id)
{
}

void FlashButton::addButtonLayer(const ButtonLayer& layer)
{
	m_layers.push_back(layer);
}

const FlashButton::button_layers_t& FlashButton::getButtonLayers() const
{
	return m_layers;
}

void FlashButton::addButtonCondition(const ButtonCondition& condition)
{
	m_conditions.push_back(condition);
}

const FlashButton::button_conditions_t& FlashButton::getButtonConditions() const
{
	return m_conditions;
}

Ref< FlashCharacterInstance > FlashButton::createInstance(ActionContext* context, FlashCharacterInstance* parent) const
{
	return gc_new< FlashButtonInstance >(context, parent, this);
}

	}
}
