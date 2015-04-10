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
	virtual bool enumerate(std::map< std::wstring, int32_t >& outStats);

	virtual bool set(const std::wstring& statId, int32_t value);
};

	}
}

#endif	// traktor_online_LanStatistics_H
