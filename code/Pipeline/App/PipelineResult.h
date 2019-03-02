#pragma once

#include "Core/Serialization/ISerializable.h"

namespace traktor
{

class PipelineResult : public ISerializable
{
	T_RTTI_CLASS;

public:
	PipelineResult();

	PipelineResult(int32_t result);

	int32_t getResult() const { return m_result; }

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_result;
};

}

