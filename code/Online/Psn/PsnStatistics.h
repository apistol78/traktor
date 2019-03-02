#pragma once

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

