/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_LocalGroup_H
#define traktor_db_LocalGroup_H

#include "Database/Provider/IProviderGroup.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace db
	{

class Context;

/*! \brief Local group.
 * \ingroup Database
 */
class LocalGroup : public IProviderGroup
{
	T_RTTI_CLASS;

public:
	LocalGroup(Context* contex, const Path& groupPath);

	virtual std::wstring getName() const T_OVERRIDE T_FINAL;

	virtual bool rename(const std::wstring& name) T_OVERRIDE T_FINAL;

	virtual bool remove() T_OVERRIDE T_FINAL;

	virtual Ref< IProviderGroup > createGroup(const std::wstring& groupName) T_OVERRIDE T_FINAL;

	virtual Ref< IProviderInstance > createInstance(const std::wstring& instanceName, const Guid& instanceGuid) T_OVERRIDE T_FINAL;

	virtual bool getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances) T_OVERRIDE T_FINAL;

private:
	Ref< Context > m_context;
	Path m_groupPath;
};

	}
}

#endif	// traktor_db_LocalGroup_H
