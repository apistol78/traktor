/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/AttachmentResult.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.AttachmentResult", AttachmentResult, Result)

void AttachmentResult::succeed(ISerializable* attachment)
{
	m_attachment = attachment;
	Result::succeed();
}

ISerializable* AttachmentResult::get() const
{
	wait();
	return m_attachment;
}

}
