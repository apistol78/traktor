/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_CompactGroup_H
#define traktor_db_CompactGroup_H

#include "Database/Provider/IProviderGroup.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace db
	{

class CompactContext;
class CompactGroupEntry;
class CompactInstance;

/*! \brief Compact group
 * \ingroup Database
 */
class CompactGroup : public IProviderGroup
{
	T_RTTI_CLASS;

public:
	CompactGroup(CompactContext* context);

	bool internalCreate(CompactGroupEntry* groupEntry);

	virtual std::wstring getName() const override final;

	virtual bool rename(const std::wstring& name) override final;

	virtual bool remove() override final;

	virtual Ref< IProviderGroup > createGroup(const std::wstring& groupName) override final;

	virtual Ref< IProviderInstance > createInstance(const std::wstring& instanceName, const Guid& instanceGuid) override final;

	virtual bool getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances) override final;

private:
	Ref< CompactContext > m_context;
	Ref< CompactGroupEntry > m_groupEntry;
	RefArray< CompactGroup > m_childGroups;
	RefArray< CompactInstance > m_childInstances;
};

	}
}

#endif	// traktor_db_CompactGroup_H
