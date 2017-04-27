/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_PhysicalAccess_H
#define traktor_db_PhysicalAccess_H

#include <string>
#include "Core/Ref.h"
#include "Core/Rtti/ITypedObject.h"

namespace traktor
{

class Path;
class ISerializable;

	namespace db
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
}

#endif	// traktor_db_PhysicalAccess_H
