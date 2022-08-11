#pragma once

#include "Core/Ref.h"

namespace traktor
{
	namespace db
	{

class BlockFile;
class CompactRegistry;

/*! Compact database context
 * \ingroup Database
 */
class CompactContext
{
public:
	CompactContext() = default;

	explicit CompactContext(BlockFile* blockFile, CompactRegistry* registry);

	BlockFile* getBlockFile() { return m_blockFile; }

	CompactRegistry* getRegistry() { return m_registry; }

private:
	Ref< BlockFile > m_blockFile;
	Ref< CompactRegistry > m_registry;
};

	}
}

