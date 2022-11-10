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
#include "Spark/Debug/CaptureControl.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.CaptureControl", 0, CaptureControl, ISerializable)

CaptureControl::CaptureControl()
:	m_frameCount(0)
{
}

CaptureControl::CaptureControl(int32_t frameCount)
:	m_frameCount(frameCount)
{
}

void CaptureControl::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"frameCount", m_frameCount);
}

	}
}
