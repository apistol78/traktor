#ifndef traktor_update_Process_H
#define traktor_update_Process_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UPDATE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class Url;

	}

	namespace update
	{

/*! \brief Update process.
 * \ingroup Update
 */
class T_DLLCLASS Process : public Object
{
	T_RTTI_CLASS;

public:
	enum CheckResult
	{
		CrUpToDate,
		CrTerminate,
		CrAborted,
		CrFailed
	};

	CheckResult check(const net::Url& bundleUrl);
};

	}
}

#endif	// traktor_update_Process_H
