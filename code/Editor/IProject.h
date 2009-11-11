#ifndef traktor_editor_IProject_H
#define traktor_editor_IProject_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace editor
	{

class Settings;

/*! \brief Project base interface.
 * \ingroup Editor
 */
class T_DLLCLASS IProject : public Object
{
	T_RTTI_CLASS(IProject)

public:
	/*! \brief Get project settings. */
	virtual Ref< Settings > getSettings() = 0;

	/*! \brief Get source asset database. */
	virtual Ref< db::Database > getSourceDatabase() = 0;

	/*! \brief Get output database. */
	virtual Ref< db::Database > getOutputDatabase() = 0;
};

	}
}

#endif	// traktor_editor_IProject_H
