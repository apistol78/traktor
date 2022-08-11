#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace db
	{

class IFileStore;

/*! Local database context.
 * \ingroup Database
 */
class Context : public Object
{
	T_RTTI_CLASS;

public:
	Context() = default;

	explicit Context(bool preferBinary, IFileStore* fileStore);

	const Guid& getSessionGuid() const;

	bool preferBinary() const;

	IFileStore* getFileStore() const;

private:
	Guid m_sessionGuid;
	bool m_preferBinary = false;
	Ref< IFileStore > m_fileStore;
};

	}
}
