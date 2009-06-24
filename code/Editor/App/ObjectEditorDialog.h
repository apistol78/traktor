#ifndef traktor_editor_ObjectEditorDialog_H
#define traktor_editor_ObjectEditorDialog_H

#include "Core/Heap/Ref.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{

class Serializable;

	namespace db
	{

class Instance;

	}

	namespace editor
	{

class Settings;
class IObjectEditor;

class ObjectEditorDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS(ObjectEditorDialog)

public:
	ObjectEditorDialog(Settings* settings, IObjectEditor* objectEditor);

	bool create(ui::Widget* parent, db::Instance* instance, Serializable* object);

	void destroy();

	bool apply(bool keep);

	void cancel();

private:
	Ref< Settings > m_settings;
	Ref< IObjectEditor > m_objectEditor;
	Ref< db::Instance > m_instance;
	Ref< Serializable > m_object;

	void eventClick(ui::Event* event);

	void eventClose(ui::Event* event);
};

	}
}

#endif	// traktor_editor_ObjectEditorDialog_H
