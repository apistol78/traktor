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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class File;
class ISerializable;
class IStream;
class Path;

}

namespace traktor::db
{

class Database;

}

namespace traktor::editor
{

/*! \Pipeline common interface.
 * \ingroup Editor
 */
class T_DLLCLASS IPipelineCommon : public Object
{
	T_RTTI_CLASS;

public:
	virtual db::Database* getSourceDatabase() const = 0;

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) = 0;

	template < typename T >
	Ref< const T > getObjectReadOnly(const Guid& guid) {
		return dynamic_type_cast< const T* >(getObjectReadOnly(guid));
	}
};

}
