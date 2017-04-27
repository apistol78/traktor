/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_PsnStatistics_H
#define traktor_online_PsnStatistics_H

#include "Online/Provider/IStatisticsProvider.h"

namespace traktor
{
	namespace online
	{

class PsnStatistics : public IStatisticsProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::map< std::wstring, int32_t >& outStats);

	virtual bool set(const std::wstring& statId, int32_t value);
};

	}
}

#endif	// traktor_online_PsnStatistics_H
