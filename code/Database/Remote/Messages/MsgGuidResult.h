/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_MsgGuidResult_H
#define traktor_db_MsgGuidResult_H

#include "Database/Remote/IMessage.h"
#include "Core/Guid.h"

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

/*! \brief Guid result.
 * \ingroup Database
 */
class T_DLLCLASS MsgGuidResult : public IMessage
{
	T_RTTI_CLASS;

public:
	MsgGuidResult(const Guid& value = Guid());

	const Guid& get() const { return m_value; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Guid m_value;
};

	}
}

#endif	// traktor_db_MsgGuidResult_H