/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_ScriptProfilerCallMeasured_H
#define traktor_amalgam_ScriptProfilerCallMeasured_H

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Script profiler measurement from running target.
 * \ingroup Amalgam
 */
class T_DLLCLASS ScriptProfilerCallMeasured : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptProfilerCallMeasured();

	ScriptProfilerCallMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration);

	const Guid& getScriptId() const { return m_scriptId; }

	const std::wstring& getFunction() const { return m_function; }

	uint32_t getCallCount() const { return m_callCount; }

	double getInclusiveDuration() const { return m_inclusiveDuration; }

	double getExclusiveDuration() const { return m_exclusiveDuration; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Guid m_scriptId;
	std::wstring m_function;
	uint32_t m_callCount;
	double m_inclusiveDuration;
	double m_exclusiveDuration;
};

	}
}

#endif	// traktor_amalgam_ScriptProfilerCallMeasured_H
