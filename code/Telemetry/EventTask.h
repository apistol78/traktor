#pragma once

#include <string>
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

	virtual TaskResult execute() override final;

private:
	std::wstring m_serverHost;
	std::wstring m_client;
	std::wstring m_symbol;
	uint64_t m_timeStamp;
};

	}
}
