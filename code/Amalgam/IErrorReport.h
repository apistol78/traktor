#ifndef traktor_amalgam_IErrorReport_H
#define traktor_amalgam_IErrorReport_H

#include "Core/Object.h"

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

class T_DLLCLASS IErrorReport : public Object
{
	T_RTTI_CLASS;

public:
	virtual void showMessage(const std::wstring& message) = 0;
};

	}
}

#endif	// traktor_amalgam_IErrorReport_H
