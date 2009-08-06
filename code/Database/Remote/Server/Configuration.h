#ifndef traktor_db_Configuration_H
#define traktor_db_Configuration_H

#include <map>
#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_SERVER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief
 */
class T_DLLCLASS Configuration : public Serializable
{
	T_RTTI_CLASS(Configuration)

public:
	uint16_t getListenPort() const;

	std::wstring getDatabaseManifest(const std::wstring& name) const;

	virtual bool serialize(Serializer& s);

private:
	uint16_t m_listenPort;
	std::map< std::wstring, std::wstring > m_databaseManifests;
};

	}
}

#endif	// traktor_db_Configuration_H
