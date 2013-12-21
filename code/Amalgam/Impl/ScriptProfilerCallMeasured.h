#ifndef traktor_amalgam_ScriptProfilerCallMeasured_H
#define traktor_amalgam_ScriptProfilerCallMeasured_H

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

class T_DLLCLASS ScriptProfilerCallMeasured : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptProfilerCallMeasured();

	ScriptProfilerCallMeasured(const std::wstring& function, double timeStamp, double inclusiveDuration, double exclusiveDuration);

	const std::wstring& getFunction() const { return m_function; }

	double getTimeStamp() const { return m_timeStamp; }

	double getInclusiveDuration() const { return m_inclusiveDuration; }

	double getExclusiveDuration() const { return m_exclusiveDuration; }

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_function;
	double m_timeStamp;
	double m_inclusiveDuration;
	double m_exclusiveDuration;
};

	}
}

#endif	// traktor_amalgam_ScriptProfilerCallMeasured_H
