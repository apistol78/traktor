/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Compact/CompactContext.h"

namespace traktor::db
{

CompactContext::CompactContext(BlockFile* blockFile, CompactRegistry* registry)
:	m_blockFile(blockFile)
,	m_registry(registry)
{
}

}
