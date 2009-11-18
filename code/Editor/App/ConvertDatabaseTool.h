#ifndef traktor_editor_EditBatchTool_H
#define traktor_editor_EditBatchTool_H

#include "Editor/IEditorTool.h"

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

class ConvertDatabaseTool : public IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const;

	virtual bool launch(ui::Widget* parent, IEditor* editor);

private:
	void threadConvert(db::Database* sourceDb, db::Database* targetDb, ui::custom::BackgroundWorkerStatus& status);
};

	}
}

#endif	// traktor_editor_EditBatchTool_H
