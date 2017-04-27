/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_CompactContext_H
#define traktor_db_CompactContext_H

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
	T_RTTI_CLASS;

public:
	CompactContext(BlockFile* blockFile, CompactRegistry* registry);

	inline Ref< BlockFile > getBlockFile() { return m_blockFile; }

	inline Ref< CompactRegistry > getRegistry() { return m_registry; }

private:
	Ref< BlockFile > m_blockFile;
	Ref< CompactRegistry > m_registry;
};

	}
}

#endif	// traktor_db_CompactContext_H
