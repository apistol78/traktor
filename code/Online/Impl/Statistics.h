/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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
	virtual bool ready() const override final;

	virtual bool enumerate(std::set< std::wstring >& outStatIds) const override final;

	virtual bool get(const std::wstring& statId, int32_t& outValue) const override final;

	virtual Ref< Result > set(const std::wstring& statId, int32_t value) override final;

	virtual Ref< Result > add(const std::wstring& statId, int32_t valueDelta) override final;

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

