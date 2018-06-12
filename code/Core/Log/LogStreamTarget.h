#ifndef traktor_LogStreamTarget_H
#define traktor_LogStreamTarget_H

#include "Core/Ref.h"
#include "Core/Log/Log.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class OutputStream;

class T_DLLCLASS LogStreamTarget : public ILogTarget
{
public:
	LogStreamTarget(OutputStream* stream);

	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL;

private:
	Ref< OutputStream > m_stream;
};

}

#endif	// traktor_LogStreamTarget_H
