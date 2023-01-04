/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Sequence.h"
#include "Spray/SequenceInstance.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Sequence", Sequence, Object)

Sequence::Sequence(const AlignedVector< Key >& keys)
:	m_keys(keys)
{
}

Ref< SequenceInstance > Sequence::createInstance() const
{
	return new SequenceInstance(this);
}

}
