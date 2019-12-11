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
class ActionWriteObject : public Action
{
	T_RTTI_CLASS;

public:
	ActionWriteObject(const Path& instancePath, const std::wstring& primaryTypeName);

	virtual bool execute(Context* context) override final;

	virtual bool undo(Context* context) override final;

	virtual void clean(Context* context) override final;

	virtual bool redundant(const Action* action) const override final;

	const AlignedVector< uint8_t >& getBuffer() const { return m_objectBuffer; }

	const Ref< IStream >& getStream() const { return m_objectStream; }

private:
	Path m_instancePath;
	std::wstring m_primaryTypeName;
	AlignedVector< uint8_t > m_objectBuffer;
	Ref< IStream > m_objectStream;
	bool m_editObject;
	bool m_editMeta;
};

	}
}

