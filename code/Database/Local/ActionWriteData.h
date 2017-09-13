/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_ActionWriteData_H
#define traktor_db_ActionWriteData_H

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
class ActionWriteData : public Action
{
	T_RTTI_CLASS;

public:
	ActionWriteData(const Path& instancePath, const std::wstring& dataName);

	virtual bool execute(Context* context) T_OVERRIDE T_FINAL;

	virtual bool undo(Context* context) T_OVERRIDE T_FINAL;

	virtual void clean(Context* context) T_OVERRIDE T_FINAL;

	virtual bool redundant(const Action* action) const T_OVERRIDE T_FINAL;

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

#endif	// traktor_db_ActionWriteData_H
