#ifndef traktor_online_SaveGameLocal_H
#define traktor_online_SaveGameLocal_H

#include "Online/ISaveGame.h"

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

class T_DLLCLASS SaveGameLocal : public ISaveGame
{
	T_RTTI_CLASS;

public:
	SaveGameLocal(sql::IConnection* db, int32_t id, const std::wstring& name);

	virtual std::wstring getName() const;

	virtual Ref< ISerializable > getAttachment() const;

private:
	Ref< sql::IConnection > m_db;
	int32_t m_id;
	std::wstring m_name;
};

	}
}

#endif	// traktor_online_SaveGameLocal_H
