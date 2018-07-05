/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_resource_ResourceCast_H
#define traktor_resource_ResourceCast_H

#include "Core/Class/Any.h"
#include "Resource/ResourceHandle.h"

namespace traktor
{

template < typename ResourceType >
struct CastAny < resource::Proxy< ResourceType >, false >
{
	static OutputStream& typeName(OutputStream& ss)
	{
		return ss << L"resource::Proxy<" << type_name< ResourceType >() << L">";
	}

	static bool accept(const Any& value)
	{
		return value.isObject< resource::ResourceHandle >();
	}

	static Any set(const resource::Proxy< ResourceType >& value)
	{
		return Any::fromObject(value.getHandle());
	}

	static resource::Proxy< ResourceType > get(const Any& value)
	{
		resource::ResourceHandle* handle = dynamic_type_cast< resource::ResourceHandle* >(value.getObject());
		if (handle)
			return resource::Proxy< ResourceType >(handle);
		else
			return resource::Proxy< ResourceType >();
	}
};

template < typename ResourceType >
struct CastAny < const resource::Proxy< ResourceType >&, false >
{
	static OutputStream& typeName(OutputStream& ss)
	{
		return ss << L"resource::Proxy<" << type_name< ResourceType >() << L">";
	}

	static bool accept(const Any& value)
	{
		return value.isObject< resource::ResourceHandle >();
	}

	static Any set(const resource::Proxy< ResourceType >& value)
	{
		return Any::fromObject(value.getHandle());
	}

	static resource::Proxy< ResourceType > get(const Any& value)
	{
		resource::ResourceHandle* handle = dynamic_type_cast< resource::ResourceHandle* >(value.getObject());
		if (handle)
			return resource::Proxy< ResourceType >(handle);
		else
			return resource::Proxy< ResourceType >();
	}
};

}

#endif	// traktor_resource_ResourceCast_H

