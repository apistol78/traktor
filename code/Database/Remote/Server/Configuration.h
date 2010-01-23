#ifndef traktor_db_Configuration_H
#define traktor_db_Configuration_H

#include <map>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_SERVER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Server configuration.
 * \ingroup Database
 */
class T_DLLCLASS Configuration : public ISerializable
{
	T_RTTI_CLASS;

public:
	Configuration();

	void setListenPort(uint16_t listenPort);

	uint16_t getListenPort() const;

	void setConnectionString(const std::wstring& name, const std::wstring& connectionString);

	std::wstring getConnectionString(const std::wstring& name) const;

	virtual bool serialize(ISerializer& s);

private:
	uint16_t m_listenPort;
	std::map< std::wstring, std::wstring > m_connectionStrings;
};

	}
}

#endif	// traktor_db_Configuration_H
