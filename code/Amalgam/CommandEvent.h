/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_CommandEvent_H
#define traktor_amalgam_CommandEvent_H

#include <string>
#include "Amalgam/IRemoteEvent.h"

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

/*! \brief Application generic command events.
 * \ingroup Amalgam
 *
 * These events are sent remotely from the editor to the
 * running target.
 */
class T_DLLCLASS CommandEvent : public IRemoteEvent
{
	T_RTTI_CLASS;

public:
	CommandEvent();

	CommandEvent(const std::wstring& function);

	const std::wstring& getFunction() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_function;
};

	}
}

#endif	// traktor_amalgam_CommandEvent_H

