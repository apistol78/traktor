/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_telemetry_EventTask_H
#define traktor_telemetry_EventTask_H

#include "Telemetry/ITask.h"

namespace traktor
{
	namespace telemetry
	{

/*! \brief
 * \ingroup Telemetry
 */
class EventTask : public ITask
{
	T_RTTI_CLASS;

public:
	EventTask(const std::wstring& serverHost, const std::wstring& client, const std::wstring& symbol);

	virtual TaskResult execute() T_OVERRIDE T_FINAL;

private:
	std::wstring m_serverHost;
	std::wstring m_client;
	std::wstring m_symbol;
	uint64_t m_timeStamp;
};

	}
}

#endif	// traktor_telemetry_EventTask_H
