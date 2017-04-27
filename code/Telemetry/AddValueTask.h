/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_telemetry_AddValueTask_H
#define traktor_telemetry_AddValueTask_H

#include "Telemetry/ITask.h"

namespace traktor
{
	namespace telemetry
	{

/*! \brief
 * \ingroup Telemetry
 */
class AddValueTask : public ITask
{
	T_RTTI_CLASS;

public:
	AddValueTask(const std::wstring& serverHost, const std::wstring& client, const std::wstring& symbol, int32_t delta);

	virtual TaskResult execute() T_OVERRIDE T_FINAL;

private:
	std::wstring m_serverHost;
	std::wstring m_client;
	std::wstring m_symbol;
	int32_t m_delta;
	uint64_t m_timeStamp;
};

	}
}

#endif	// traktor_telemetry_AddValueTask_H
