#ifndef traktor_online_LocalStatistics_H
#define traktor_online_LocalStatistics_H

#include "Online/Provider/IStatisticsProvider.h"

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace online
	{

class LocalStatistics : public IStatisticsProvider
{
	T_RTTI_CLASS;

public:
	LocalStatistics(sql::IConnection* db);

	virtual bool enumerate(std::map< std::wstring, float >& outStats);

	virtual bool set(const std::wstring& statId, int32_t value);

private:
	Ref< sql::IConnection > m_db;
};

	}
}

#endif	// traktor_online_LocalStatistics_H
