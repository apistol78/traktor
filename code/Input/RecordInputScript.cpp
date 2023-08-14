/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/MathUtils.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Input/RecordInputScript.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.RecordInputScript", 0, RecordInputScript, ISerializable)

void RecordInputScript::addInputValue(uint32_t frame, int control, float value)
{
	std::vector< Input >& data = m_data[control];
	if (!data.empty() && data.back().value == value)
	{
		data.back().end = frame;
		return;
	}
	Input input = { frame, frame, value };
	data.push_back(input);
}

float RecordInputScript::getInputValue(uint32_t frame, int control)
{
	std::vector< Input >& data = m_data[control];
	for (std::vector< Input >::iterator i = data.begin(); i != data.end(); ++i)
	{
		if (frame >= i->start && frame <= i->end)
			return i->value;
	}
	return 0.0f;
}

uint32_t RecordInputScript::getLastFrame() const
{
	uint32_t last = 0;
	for (std::map< int, std::vector< Input > >::const_iterator i = m_data.begin(); i != m_data.end(); ++i)
	{
		if (!i->second.empty())
			last = max(last, i->second.back().end);
	}
	return last;
}

void RecordInputScript::serialize(ISerializer& s)
{
	s >> MemberStlMap<
		int,
		std::vector< Input >,
		Member< int >,
		MemberStlVector<
			Input,
			MemberComposite< Input >
		>
	>(L"data", m_data);
}

void RecordInputScript::Input::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"start", start);
	s >> Member< uint32_t >(L"end", end);
	s >> Member< float >(L"value", value);
}

}
