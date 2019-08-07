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
class SetValueTask : public ITask
{
	T_RTTI_CLASS;

public:
	SetValueTask(const std::wstring& serverHost, const std::wstring& client, uint32_t sequenceNr, const std::wstring& symbol, int32_t value);

	virtual TaskResult execute() override final;

private:
	std::wstring m_serverHost;
	std::wstring m_client;
	uint32_t m_sequenceNr;
	std::wstring m_symbol;
	int32_t m_value;
	uint64_t m_timeStamp;
};

	}
}
