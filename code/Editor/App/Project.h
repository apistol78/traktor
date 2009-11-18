#ifndef traktor_editor_Project_H
#define traktor_editor_Project_H

#include "Core/Io/Path.h"
#include "Editor/IProject.h"

namespace traktor
{
	namespace editor
	{

/*! \brief Project implementation.
 * \ingroup Editor
 */
class Project : public IProject
{
	T_RTTI_CLASS;

public:
	bool create(const Path& fileName);

	bool open(const Path& fileName);

	void close();

	/*! \name IProject implementation */
	//@{

	virtual Ref< Settings > getSettings();

	virtual Ref< db::Database > getSourceDatabase();

	virtual Ref< db::Database > getOutputDatabase();

	//@}

private:
	traktor::Ref< Settings > m_settings;
	traktor::Ref< db::Database > m_sourceDatabase;
	traktor::Ref< db::Database > m_outputDatabase;
};

	}
}

#endif	// traktor_editor_Project_H
