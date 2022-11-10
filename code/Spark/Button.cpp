/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Spark/Button.h"
#include "Spark/ButtonInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.Button", 0, Button, Character)

void Button::addButtonLayer(const ButtonLayer& layer)
{
	m_layers.push_back(layer);
}

const Button::button_layers_t& Button::getButtonLayers() const
{
	return m_layers;
}

Ref< CharacterInstance > Button::createInstance(
	Context* context,
	Dictionary* dictionary,
	CharacterInstance* parent,
	const std::string& name,
	const Matrix33& transform
) const
{
	return new ButtonInstance(context, dictionary, parent, this);
}

void Button::serialize(ISerializer& s)
{
	Character::serialize(s);

	s >> MemberAlignedVector< ButtonLayer, MemberComposite< ButtonLayer > >(L"layers", m_layers);
}

void Button::ButtonLayer::serialize(ISerializer& s)
{
	s >> Member< uint8_t >(L"state", state);
	s >> Member< uint16_t >(L"characterId", characterId);
	s >> Member< uint16_t >(L"placeDepth", placeDepth);
	s >> Member< Matrix33 >(L"placeMatrix", placeMatrix);
	s >> MemberComposite< ColorTransform >(L"cxform", cxform);
}

	}
}
