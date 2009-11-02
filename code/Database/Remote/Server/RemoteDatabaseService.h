#ifndef traktor_db_RemoteDatabaseService_H
#define traktor_db_RemoteDatabaseService_H

#include <vector>
#include "Net/Discovery/IService.h"

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

/*! \brief Network discovery service.
 * \ingroup Database
 */
class T_DLLCLASS RemoteDatabaseService : public net::IService
{
	T_RTTI_CLASS(RemoteDatabaseService)

public:
	RemoteDatabaseService();
	
	RemoteDatabaseService(const std::wstring& host, uint16_t port);
	
	const std::wstring& getHost() const;
	
	uint16_t getPort() const;
	
	virtual std::wstring getDescription() const;
	
	virtual bool serialize(Serializer& s);
	
private:
	std::wstring m_host;
	uint16_t m_port;
};
	
	}
}

#endif	// traktor_db_RemoteDatabaseService_H
