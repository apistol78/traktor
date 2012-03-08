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
	virtual bool enumerate(std::map< std::wstring, float >& outStats);

	virtual bool set(const std::wstring& statId, float value);
};

	}
}

#endif	// traktor_online_GcStatistics_H
