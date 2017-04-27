/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_MsgEpochResult_H
#define traktor_db_MsgEpochResult_H

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

/*! \brief Epoch value result.
 * \ingroup Database
 */
class T_DLLCLASS MsgEpochResult : public IMessage
{
	T_RTTI_CLASS;

public:
	MsgEpochResult(uint64_t value = 0);

	uint64_t get() const { return m_value; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	uint64_t m_value;
};

	}
}

#endif	// traktor_db_MsgEpochResult_H