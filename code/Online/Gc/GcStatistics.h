/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_GcStatistics_H
#define traktor_online_GcStatistics_H

#include "Online/Provider/IStatisticsProvider.h"

namespace traktor
{
	namespace online
	{

class GcStatistics : public IStatisticsProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::map< std::wstring, float >& outStats) T_OVERRIDE T_FINAL;

	virtual bool set(const std::wstring& statId, float value) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_online_GcStatistics_H
