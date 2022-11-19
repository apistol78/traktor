/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"

namespace traktor::db
{

class BlockFile;
class CompactRegistry;

/*! Compact database context
 * \ingroup Database
 */
class CompactContext
{
public:
	CompactContext() = default;

	explicit CompactContext(BlockFile* blockFile, CompactRegistry* registry);

	BlockFile* getBlockFile() { return m_blockFile; }

	CompactRegistry* getRegistry() { return m_registry; }

private:
	Ref< BlockFile > m_blockFile;
	Ref< CompactRegistry > m_registry;
};

}

