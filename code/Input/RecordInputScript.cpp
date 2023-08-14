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
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Input/RecordInputScript.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.RecordInputScript", 0, RecordInputScript, ISerializable)

void RecordInputScript::addInputValue(uint32_t frame, int control, float value)
{
	AlignedVector< Input >& data = m_data[control];
	if (!data.empty() && data.back().value == value)
	{
		data.back().end = frame;
		return;
	}
	Input input = { frame, frame, value };
	data.push_back(input);
}

float RecordInputScript::getInputValue(uint32_t frame, int control) const
{
	for (const auto& data : m_data[control])
	{
		if (frame >= data.start && frame <= data.end)
			return data.value;
	}
	return 0.0f;
}

uint32_t RecordInputScript::getLastFrame() const
{
	uint32_t last = 0;
	for (auto it : m_data)
	{
		if (!it.second.empty())
			last = max(last, it.second.back().end);
	}
	return last;
}

void RecordInputScript::serialize(ISerializer& s)
{
	s >> MemberSmallMap<
		int,
		AlignedVector< Input >,
		Member< int >,
		MemberAlignedVector<
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
