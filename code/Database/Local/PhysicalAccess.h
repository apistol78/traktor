/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Rtti/ITypedObject.h"

namespace traktor
{

class Path;
class ISerializable;

}

namespace traktor::db
{

Path getInstanceObjectPath(const Path& instancePath);

Path getInstanceMetaPath(const Path& instancePath);

Path getInstanceDataPath(const Path& instancePath, const std::wstring& dataName);

Ref< ISerializable > readPhysicalObject(const Path& objectPath);

bool writePhysicalObject(const Path& objectPath, const ISerializable* object, bool binary);

template < typename ObjectType >
Ref< ObjectType > readPhysicalObject(const Path& objectPath)
{
	return dynamic_type_cast< ObjectType* >(readPhysicalObject(objectPath));
}

}
