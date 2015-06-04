#ifndef traktor_amalgam_TargetLog_H
#define traktor_amalgam_TargetLog_H

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

class T_DLLCLASS TargetLog : public ISerializable
{
	T_RTTI_CLASS;

public:
	TargetLog();

	TargetLog(int32_t level, const std::wstring& text);

	int32_t getLevel() const { return m_level; }

	const std::wstring& getText() const { return m_text; }

	virtual void serialize(ISerializer& s);

private:
	int32_t m_level;
	std::wstring m_text;
};

	}
}

#endif	// traktor_amalgam_TargetLog_H
