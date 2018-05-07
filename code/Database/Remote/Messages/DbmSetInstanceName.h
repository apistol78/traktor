/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_DbmSetInstanceName_H
#define traktor_db_DbmSetInstanceName_H

#include <string>
#include "Database/Remote/IMessage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief
 */
class T_DLLCLASS DbmSetInstanceName : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmSetInstanceName(uint32_t handle = 0, const std::wstring& name = L"");

	uint32_t getHandle() const { return m_handle; }

	const std::wstring& getName() const { return m_name; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	uint32_t m_handle;
	std::wstring m_name;
};

	}
}

#endif	// traktor_db_DbmSetInstanceName_H
