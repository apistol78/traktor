/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_IStatistics_H
#define traktor_online_IStatistics_H

#include <string>
#include "Online/Result.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class T_DLLCLASS IStatistics : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const = 0;

	virtual bool enumerate(std::set< std::wstring >& outStatIds) const = 0;

	virtual bool get(const std::wstring& statId, int32_t& outValue) const = 0;

	virtual Ref< Result > set(const std::wstring& statId, int32_t value) = 0;

	virtual Ref< Result > add(const std::wstring& statId, int32_t valueDelta) = 0;
};

	}
}

#endif	// traktor_online_IStatistics_H
