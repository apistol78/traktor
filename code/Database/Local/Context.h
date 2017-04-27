/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_Context_H
#define traktor_db_Context_H

#include "Core/Object.h"
#include "Core/Guid.h"

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

#endif	// traktor_db_Context_H
