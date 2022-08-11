#pragma once

#include "Core/Io/Path.h"
#include "Database/Local/Action.h"

namespace traktor
{

class IStream;

	namespace db
	{

/*! Transaction remove all data action.
 * \ingroup Database
 */
class ActionRemoveAllData : public Action
{
	T_RTTI_CLASS;

public:
	explicit ActionRemoveAllData(const Path& instancePath);

	virtual bool execute(Context& context) override final;

	virtual bool undo(Context& context) override final;

	virtual void clean(Context& context) override final;

	virtual bool redundant(const Action* action) const override final;

private:
	Path m_instancePath;
	std::vector< std::wstring > m_renamedFiles;
};

	}
}

