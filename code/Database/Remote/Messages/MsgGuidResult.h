#pragma once

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

/*! Guid result.
 * \ingroup Database
 */
class T_DLLCLASS MsgGuidResult : public IMessage
{
	T_RTTI_CLASS;

public:
	MsgGuidResult(const Guid& value = Guid());

	const Guid& get() const { return m_value; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_value;
};

	}
}

