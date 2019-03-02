#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace db
	{

class IFileStore;

/*! \brief Local database context.
 * \ingroup Database
 */
class Context : public Object
{
	T_RTTI_CLASS;

public:
	Context(bool preferBinary, IFileStore* fileStore);

	const Guid& getSessionGuid() const;

	bool preferBinary() const;

	IFileStore* getFileStore() const;

private:
	Guid m_sessionGuid;
	bool m_preferBinary;
	Ref< IFileStore > m_fileStore;
};

	}
}

