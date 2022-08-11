#pragma once

#include "Database/Provider/IProviderGroup.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace db
	{

class CompactContext;
class CompactGroupEntry;
class CompactInstance;

/*! Compact group
 * \ingroup Database
 */
class CompactGroup : public IProviderGroup
{
	T_RTTI_CLASS;

public:
	explicit CompactGroup(CompactContext& context);

	bool internalCreate(CompactGroupEntry* groupEntry);

	virtual std::wstring getName() const override final;

	virtual bool rename(const std::wstring& name) override final;

	virtual bool remove() override final;

	virtual Ref< IProviderGroup > createGroup(const std::wstring& groupName) override final;

	virtual Ref< IProviderInstance > createInstance(const std::wstring& instanceName, const Guid& instanceGuid) override final;

	virtual bool getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances) override final;

private:
	CompactContext& m_context;
	Ref< CompactGroupEntry > m_groupEntry;
	RefArray< CompactGroup > m_childGroups;
	RefArray< CompactInstance > m_childInstances;
};

	}
}

