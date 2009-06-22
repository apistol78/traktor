#ifndef traktor_db_Context_H
#define traktor_db_Context_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace db
	{

/*! \brief Local database context.
 * \ingroup Database
 */
class Context : public Object
{
	T_RTTI_CLASS(Context)

public:
	Context(bool preferBinary);

	const Guid& getSessionGuid() const;

	bool preferBinary() const;

private:
	Guid m_sessionGuid;
	bool m_preferBinary;
};

	}
}

#endif	// traktor_db_Context_H
