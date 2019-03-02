#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

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
	T_RTTI_CLASS;

public:
	CompactContext(BlockFile* blockFile, CompactRegistry* registry);

	BlockFile* getBlockFile() { return m_blockFile; }

	CompactRegistry* getRegistry() { return m_registry; }

private:
	Ref< BlockFile > m_blockFile;
	Ref< CompactRegistry > m_registry;
};

	}
}

