/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_PipelineResult_H
#define traktor_PipelineResult_H

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

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	int32_t m_result;
};

}

#endif	// traktor_PipelineResult_H
