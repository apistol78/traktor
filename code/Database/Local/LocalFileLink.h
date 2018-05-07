/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_LocalFileLink_H
#define traktor_db_LocalFileLink_H

#include <string>
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace db
	{

/*! \brief Virtual file link.
 * \ingroup Database
 */
class LocalFileLink : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	inline const std::wstring& getPath() const { return m_path; }

private:
	std::wstring m_path;
};

	}
}

#endif	// traktor_db_LocalFileLink_H
