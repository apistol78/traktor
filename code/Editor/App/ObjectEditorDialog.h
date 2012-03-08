#ifndef traktor_editor_ObjectEditorDialog_H
#define traktor_editor_ObjectEditorDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{

class ISerializable;
class PropertyGroup;

	namespace db
	{

class Instance;

	}

	namespace editor
	{

class IObjectEditor;

class ObjectEditorDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	ObjectEditorDialog(PropertyGroup* settings, IObjectEditor* objectEditor);

	bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object);

	void destroy();

	bool apply(bool keep);

	void cancel();

private:
	Ref< PropertyGroup > m_settings;
	Ref< IObjectEditor > m_objectEditor;
	Ref< db::Instance > m_instance;
	Ref< ISerializable > m_object;

	void eventClick(ui::Event* event);

	void eventClose(ui::Event* event);
};

	}
}

#endif	// traktor_editor_ObjectEditorDialog_H
