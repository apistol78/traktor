/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Image2/IImgStep.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_VERSION_CLASS(L"traktor.render.IImgStep", 1, IImgStep, ISerializable)

void IImgStep::serialize(ISerializer& s)
{
	if (s.getVersion< IImgStep >() >= 1)
		s >> Member< std::wstring >(L"name", m_name);
}

}
