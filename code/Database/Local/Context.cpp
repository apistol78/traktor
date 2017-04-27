/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Local/Context.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Context", Context, Object)

Context::Context(bool preferBinary, IFileStore* fileStore)
:	m_sessionGuid(Guid::create())
,	m_preferBinary(preferBinary)
,	m_fileStore(fileStore)
{
}

const Guid& Context::getSessionGuid() const
{
	return m_sessionGuid;
}

bool Context::preferBinary() const
{
	return m_preferBinary;
}

IFileStore* Context::getFileStore() const
{
	return m_fileStore;
}

	}
}
