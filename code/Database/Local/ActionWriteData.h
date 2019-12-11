#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Path.h"
#include "Database/Local/Action.h"

namespace traktor
{

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
	ActionWriteData(const Path& instancePath, const std::wstring& dataName);

	virtual bool execute(Context* context) override final;

	virtual bool undo(Context* context) override final;

	virtual void clean(Context* context) override final;

	virtual bool redundant(const Action* action) const override final;

	const std::wstring& getName() const { return m_dataName; }

	const AlignedVector< uint8_t >& getBuffer() const { return m_dataBuffer; }

	const Ref< IStream >& getStream() const { return m_dataStream; }

private:
	Path m_instancePath;
	std::wstring m_dataName;
	AlignedVector< uint8_t > m_dataBuffer;
	Ref< IStream > m_dataStream;
	bool m_existingBlob;
};

	}
}

