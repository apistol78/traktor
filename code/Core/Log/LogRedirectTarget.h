#ifndef traktor_LogRedirectTarget_H
#define traktor_LogRedirectTarget_H

#include "Core/RefArray.h"
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

class T_DLLCLASS LogRedirectTarget : public ILogTarget
{
public:
	explicit LogRedirectTarget(ILogTarget* target);

	explicit LogRedirectTarget(ILogTarget* target1, ILogTarget* target2);

	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL;

private:
	RefArray< ILogTarget > m_targets;
};

}

#endif	// traktor_LogRedirectTarget_H
