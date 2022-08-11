#pragma once

#include "Core/Io/Path.h"
#include "Database/Local/Action.h"

namespace traktor
{

class ChunkMemory;
class IStream;

	namespace db
	{

/*! Transaction write object action.
 * \ingroup Database
 */
class ActionWriteData : public Action
{
	T_RTTI_CLASS;

public:
	explicit ActionWriteData(const Path& instancePath, const std::wstring& dataName);

	virtual bool execute(Context& context) override final;

	virtual bool undo(Context& context) override final;

	virtual void clean(Context& context) override final;

	virtual bool redundant(const Action* action) const override final;

	const std::wstring& getName() const { return m_dataName; }

	Ref< IStream > getWriteStream() const;

	Ref< IStream > getReadStream() const;

private:
	Path m_instancePath;
	std::wstring m_dataName;
	Ref< ChunkMemory > m_dataMemory;
	bool m_existingBlob;
};

	}
}

