/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Flash/Button.h"
#include "Flash/ButtonInstance.h"
#include "Flash/SwfMembers.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.Button", 0, Button, Character)

Button::Button()
{
}

Button::Button(uint16_t id)
:	Character(id)
{
}

void Button::addButtonLayer(const ButtonLayer& layer)
{
	m_layers.push_back(layer);
}

const Button::button_layers_t& Button::getButtonLayers() const
{
	return m_layers;
}

void Button::addButtonCondition(const ButtonCondition& condition)
{
	m_conditions.push_back(condition);
}

const Button::button_conditions_t& Button::getButtonConditions() const
{
	return m_conditions;
}

Ref< CharacterInstance > Button::createInstance(
	ActionContext* context,
	Dictionary* dictionary,
	CharacterInstance* parent,
	const std::string& name,
	const Matrix33& transform,
	const ActionObject* initObject,
	const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
) const
{
	return new ButtonInstance(context, dictionary, parent, this);
}

void Button::serialize(ISerializer& s)
{
	Character::serialize(s);

	s >> MemberAlignedVector< ButtonLayer, MemberComposite< ButtonLayer > >(L"layers", m_layers);
	s >> MemberAlignedVector< ButtonCondition, MemberComposite< ButtonCondition > >(L"conditions", m_conditions);
}

void Button::ButtonLayer::serialize(ISerializer& s)
{
	s >> Member< uint8_t >(L"state", state);
	s >> Member< uint16_t >(L"characterId", characterId);
	s >> Member< uint16_t >(L"placeDepth", placeDepth);
	s >> Member< Matrix33 >(L"placeMatrix", placeMatrix);
	s >> MemberColorTransform(L"cxform", cxform);
}

void Button::ButtonCondition::serialize(ISerializer& s)
{
	s >> Member< uint8_t >(L"key", key);
	s >> Member< uint16_t >(L"mask", mask);
	s >> MemberRef< const IActionVMImage >(L"script", script);
}

	}
}
