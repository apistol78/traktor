#ifndef traktor_db_ActionWriteData_H
#define traktor_db_ActionWriteData_H

#include "Database/Local/Action.h"
#include "Core/Io/Path.h"

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

	virtual bool execute(Context* context);

	virtual bool undo(Context* context);

	virtual void clean(Context* context);

	virtual bool redundant(const Action* action) const;

	const Ref< IStream >& getStream() const { return m_dataStream; }

private:
	Path m_instancePath;
	std::wstring m_dataName;
	std::vector< uint8_t > m_dataBuffer;
	Ref< IStream > m_dataStream;
	bool m_existingBlob;
};

	}
}

#endif	// traktor_db_ActionWriteData_H
