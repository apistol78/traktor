/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_ActionWriteObject_H
#define traktor_db_ActionWriteObject_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Path.h"
#include "Database/Local/Action.h"

namespace traktor
{

class IStream;

	namespace db
	{

/*! \brief Transaction write object action.
 * \ingroup Database
 */
class ActionWriteObject : public Action
{
	T_RTTI_CLASS;

public:
	ActionWriteObject(const Path& instancePath, const std::wstring& primaryTypeName);

	virtual bool execute(Context* context) T_OVERRIDE T_FINAL;

	virtual bool undo(Context* context) T_OVERRIDE T_FINAL;

	virtual void clean(Context* context) T_OVERRIDE T_FINAL;

	virtual bool redundant(const Action* action) const T_OVERRIDE T_FINAL;

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

#endif	// traktor_db_ActionWriteObject_H
