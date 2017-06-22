/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_i18n_DictionaryEditorPage_H
#define traktor_i18n_DictionaryEditorPage_H

#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPageSite;

	}

	namespace ui
	{
		namespace custom
		{

class GridItemContentChangeEvent;
class GridRowDoubleClickEvent;
class GridView;
class ToolBarButtonClickEvent;

		}
	}

	namespace i18n
	{

class Dictionary;

class T_DLLCLASS DictionaryEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	DictionaryEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ui::custom::GridView > m_gridDictionary;
	Ref< Dictionary > m_dictionary;
	Ref< Dictionary > m_referenceDictionary;

	void updateGrid();

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventGridRowDoubleClick(ui::custom::GridRowDoubleClickEvent* event);

	void eventGridItemChange(ui::custom::GridItemContentChangeEvent* event);
};

	}
}

#endif	// traktor_i18n_DictionaryEditorPage_H
