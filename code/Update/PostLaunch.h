#ifndef traktor_update_PostLaunch_H
#define traktor_update_PostLaunch_H

#include "Update/IPostAction.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UPDATE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace update
	{

/*! \brief Post update launch.
 * \ingroup Update
 */
class T_DLLCLASS PostLaunch : public IPostAction
{
	T_RTTI_CLASS;

public:
	virtual bool execute() const;

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_fileName;
	std::wstring m_arguments;
};

	}
}

#endif	// traktor_update_PostLaunch_H
