#pragma once

#include "Core/Serialization/ISerializable.h"

namespace traktor
{

class PipelineLog : public ISerializable
{
	T_RTTI_CLASS;

public:
	PipelineLog();

	PipelineLog(uint32_t threadId, int32_t level, const std::wstring& text);

	uint32_t getThreadId() const { return m_threadId; }

	int32_t getLevel() const { return m_level; }

	const std::wstring& getText() const { return m_text; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_threadId;
	int32_t m_level;
	std::wstring m_text;
};

}

