#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/FlashButton.h"
#include "Flash/FlashButtonInstance.h"
#include "Flash/SwfMembers.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashButton", 0, FlashButton, FlashCharacter)

FlashButton::FlashButton()
{
}

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

Ref< FlashCharacterInstance > FlashButton::createInstance(
	ActionContext* context,
	FlashCharacterInstance* parent,
	const std::string& name,
	const Matrix33& transform,
	const ActionObject* initObject,
	const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
) const
{
	return new FlashButtonInstance(context, parent, this);
}

void FlashButton::serialize(ISerializer& s)
{
	FlashCharacter::serialize(s);

	s >> MemberAlignedVector< ButtonLayer, MemberComposite< ButtonLayer > >(L"layers", m_layers);
	s >> MemberStlVector< ButtonCondition, MemberComposite< ButtonCondition > >(L"conditions", m_conditions);
}

void FlashButton::ButtonLayer::serialize(ISerializer& s)
{
	s >> Member< uint8_t >(L"state", state);
	s >> Member< uint16_t >(L"characterId", characterId);
	s >> Member< uint16_t >(L"placeDepth", placeDepth);
	s >> Member< Matrix33 >(L"placeMatrix", placeMatrix);
	s >> MemberSwfCxTransform(L"cxform", cxform);
}

void FlashButton::ButtonCondition::serialize(ISerializer& s)
{
	s >> Member< uint8_t >(L"key", key);
	s >> Member< uint16_t >(L"mask", mask);
	s >> MemberRef< const IActionVMImage >(L"script", script);
}

	}
}
