#ifndef traktor_db_ActionWriteData_H
#define traktor_db_ActionWriteData_H

#include "Database/Local/Action.h"
#include "Core/Heap/Ref.h"
#include "Core/Io/Path.h"

namespace traktor
{

class Stream;

	namespace db
	{

/*! \brief Transaction write object action.
 * \ingroup Database
 */
class ActionWriteData : public Action
{
	T_RTTI_CLASS(Action)

public:
	ActionWriteData(const Path& instancePath, const std::wstring& dataName);

	virtual bool execute(Context* context);

	virtual bool undo(Context* context);

	virtual void clean(Context* context);

	const Ref< Stream >& getStream() const { return m_dataStream; }

private:
	Path m_instancePath;
	std::wstring m_dataName;
	std::vector< uint8_t > m_dataBuffer;
	Ref< Stream > m_dataStream;
	bool m_existingBlob;
};

	}
}

#endif	// traktor_db_ActionWriteData_H
