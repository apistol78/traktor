/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_PipelineLog_H
#define traktor_PipelineLog_H

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

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	uint32_t m_threadId;
	int32_t m_level;
	std::wstring m_text;
};

}

#endif	// traktor_PipelineLog_H
