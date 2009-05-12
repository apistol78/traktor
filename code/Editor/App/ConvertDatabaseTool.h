#ifndef traktor_editor_EditBatchTool_H
#define traktor_editor_EditBatchTool_H

#include "Editor/EditorTool.h"

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace ui
	{
		namespace custom
		{

class BackgroundWorkerStatus;

		}
	}

	namespace editor
	{

class ConvertDatabaseTool : public EditorTool
{
	T_RTTI_CLASS(ConvertDatabaseTool)

public:
	virtual std::wstring getDescription() const;

	virtual bool launch(ui::Widget* parent, Editor* editor);

private:
	void threadConvert(db::Database* sourceDb, db::Database* targetDb, ui::custom::BackgroundWorkerStatus& status);
};

	}
}

#endif	// traktor_editor_EditBatchTool_H
