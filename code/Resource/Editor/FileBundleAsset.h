/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_resource_FileBundleAsset_H
#define traktor_resource_FileBundleAsset_H

#include <list>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class T_DLLCLASS FileBundleAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Pattern
	{
		std::wstring sourceBase;
		std::wstring outputBase;
		std::wstring sourceMask;
		bool recursive;

		Pattern();

		void serialize(ISerializer& s);
	};

	const std::list< Pattern >& getPatterns() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::list< Pattern > m_patterns;
};

	}
}

#endif	// traktor_resource_FileBundleAsset_H
