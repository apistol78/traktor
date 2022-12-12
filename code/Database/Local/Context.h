/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor::db
{

class IFileStore;

/*! Local database context.
 * \ingroup Database
 */
class Context : public Object
{
	T_RTTI_CLASS;

public:
	Context() = default;

	explicit Context(bool preferBinary, IFileStore* fileStore);

	const Guid& getSessionGuid() const;

	bool preferBinary() const;

	IFileStore* getFileStore() const;

private:
	Guid m_sessionGuid;
	bool m_preferBinary = false;
	Ref< IFileStore > m_fileStore;
};

}
