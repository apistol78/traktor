#ifndef traktor_online_UserLocal_H
#define traktor_online_UserLocal_H

#include "Online/IUser.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_LOCAL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace online
	{

class UserLocal : public IUser
{
	T_RTTI_CLASS;

public:
	UserLocal(sql::IConnection* db, int32_t id, const std::wstring& name);

	virtual std::wstring getName() const;

	virtual bool getFriends(RefArray< IUser >& outFriends) const;

	virtual bool sendMessage(const std::wstring& message) const;

	int32_t getId() const { return m_id; }

private:
	Ref< sql::IConnection > m_db;
	int32_t m_id;
	std::wstring m_name;
};

	}
}

#endif	// traktor_online_UserLocal_H
