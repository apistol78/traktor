/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_Statistics_H
#define traktor_online_Statistics_H

#include <map>
#include "Online/IStatistics.h"

namespace traktor
{
	namespace online
	{

class IStatisticsProvider;
class TaskQueue;

class Statistics : public IStatistics
{
	T_RTTI_CLASS

public:
	virtual bool ready() const T_OVERRIDE T_FINAL;

	virtual bool enumerate(std::set< std::wstring >& outStatIds) const T_OVERRIDE T_FINAL;

	virtual bool get(const std::wstring& statId, int32_t& outValue) const T_OVERRIDE T_FINAL;

	virtual Ref< Result > set(const std::wstring& statId, int32_t value) T_OVERRIDE T_FINAL;

	virtual Ref< Result > add(const std::wstring& statId, int32_t valueDelta) T_OVERRIDE T_FINAL;

private:
	friend class SessionManager;

	Ref< IStatisticsProvider > m_provider;
	Ref< TaskQueue > m_taskQueue;
	mutable Semaphore m_lock;
	std::map< std::wstring, int32_t > m_statistics;
	bool m_ready;

	Statistics(IStatisticsProvider* provider, TaskQueue* taskQueue);

	void enqueueEnumeration();

	void callbackEnumStatistics(const std::map< std::wstring, int32_t >& statistics);
};

	}
}

#endif	// traktor_online_Statistics_H
