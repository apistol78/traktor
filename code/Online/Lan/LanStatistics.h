/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LanStatistics_H
#define traktor_online_LanStatistics_H

#include "Online/Provider/IStatisticsProvider.h"

namespace traktor
{
	namespace online
	{

class LanStatistics : public IStatisticsProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::map< std::wstring, int32_t >& outStats) T_OVERRIDE T_FINAL;

	virtual bool set(const std::wstring& statId, int32_t value) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_online_LanStatistics_H
