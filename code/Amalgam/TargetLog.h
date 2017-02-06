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

/*! \brief Log statement from running target.
 * \ingroup Amalgam
 */
class T_DLLCLASS TargetLog : public ISerializable
{
	T_RTTI_CLASS;

public:
	TargetLog();

	TargetLog(uint32_t threadId, int32_t level, const std::wstring& text);

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
}

#endif	// traktor_amalgam_TargetLog_H
