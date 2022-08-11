#include "Database/Compact/CompactContext.h"

namespace traktor
{
	namespace db
	{

CompactContext::CompactContext(BlockFile* blockFile, CompactRegistry* registry)
:	m_blockFile(blockFile)
,	m_registry(registry)
{
}

	}
}
