#ifndef traktor_db_CompactContext_H
#define traktor_db_CompactContext_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"

namespace traktor
{
	namespace db
	{

class BlockFile;
class CompactRegistry;

/*! \brief Compact database context
 * \ingroup Database
 */
class CompactContext : public Object
{
	T_RTTI_CLASS(CompactContext)

public:
	CompactContext(BlockFile* blockFile, CompactRegistry* registry);

	inline BlockFile* getBlockFile() { return m_blockFile; }

	inline CompactRegistry* getRegistry() { return m_registry; }

private:
	Ref< BlockFile > m_blockFile;
	Ref< CompactRegistry > m_registry;
};

	}
}

#endif	// traktor_db_CompactContext_H
