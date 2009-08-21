#ifndef traktor_i18n_DictionaryEditorPage_H
#define traktor_i18n_DictionaryEditorPage_H

#include "Core/Heap/Ref.h"
#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Event;

		namespace custom
		{

class GridView;

		}
	}

	namespace i18n
	{

class Dictionary;

class T_DLLCLASS DictionaryEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS(DictionaryEditorPage)

public:
	DictionaryEditorPage(editor::IEditor* editor);

	virtual bool create(ui::Container* parent, editor::IEditorPageSite* site);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual	bool setDataObject(db::Instance* instance, Object* data);

	virtual Object* getDataObject();

	virtual void propertiesChanged();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(const Guid& eventId);

private:
	editor::IEditor* m_editor;
	Ref< ui::custom::GridView > m_gridDictionary;
	Ref< Dictionary > m_dictionary;

	void eventGridDoubleClick(ui::Event* event);
};

	}
}

#endif	// traktor_i18n_DictionaryEditorPage_H
