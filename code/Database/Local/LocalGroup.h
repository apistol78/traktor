#pragma once

#include "Database/Provider/IProviderGroup.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace db
	{

class Context;

/*! Local group.
 * \ingroup Database
 */
class LocalGroup : public IProviderGroup
{
	T_RTTI_CLASS;

public:
	explicit LocalGroup(Context& contex, const Path& groupPath);

	virtual std::wstring getName() const override final;

	virtual bool rename(const std::wstring& name) override final;

	virtual bool remove() override final;

	virtual Ref< IProviderGroup > createGroup(const std::wstring& groupName) override final;

	virtual Ref< IProviderInstance > createInstance(const std::wstring& instanceName, const Guid& instanceGuid) override final;

	virtual bool getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances) override final;

private:
	Context& m_context;
	Path m_groupPath;
};

	}
}

