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
class AddValueTask : public ITask
{
	T_RTTI_CLASS;

public:
	AddValueTask(const std::wstring& serverHost, const std::wstring& client, const std::wstring& symbol, int32_t delta);

	virtual TaskResult execute() override final;

private:
	std::wstring m_serverHost;
	std::wstring m_client;
	std::wstring m_symbol;
	int32_t m_delta;
	uint64_t m_timeStamp;
};

	}
}
