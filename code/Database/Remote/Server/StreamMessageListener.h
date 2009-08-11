#ifndef traktor_db_StreamMessageListener_H
#define traktor_db_StreamMessageListener_H

#include "Core/Heap/Ref.h"
#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief Stream message listener.
 * \ingroup Database
 */
class StreamMessageListener : public IMessageListenerImpl< StreamMessageListener >
{
	T_RTTI_CLASS(StreamMessageListener)

public:
	StreamMessageListener(Connection* connection);

private:
	Ref< Connection > m_connection;

	bool messageClose(const class StmClose* message);

	bool messageGetStat(const class StmGetStat* message);

	bool messageSeek(const class StmSeek* message);

	bool messageRead(const class StmRead* message);

	bool messageWrite(const class StmWrite* message);
};

	}
}

#endif	// traktor_db_StreamMessageListener_H
