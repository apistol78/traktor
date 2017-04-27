/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_resource_ResourceBundle_H
#define traktor_resource_ResourceBundle_H

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class T_DLLCLASS ResourceBundle : public ISerializable
{
	T_RTTI_CLASS;

public:
	ResourceBundle();

	ResourceBundle(const std::vector< std::pair< const TypeInfo*, Guid > >& resources, bool persistent);

	const std::vector< std::pair< const TypeInfo*, Guid > >& get() const;

	bool persistent() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::vector< std::pair< const TypeInfo*, Guid > > m_resources;
	bool m_persistent;
};

	}
}

#endif	// traktor_resource_ResourceBundle_H
