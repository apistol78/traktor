/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_DefaultObjectEditor_H
#define traktor_editor_DefaultObjectEditor_H

#include "Editor/IObjectEditor.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class PropertyCommandEvent;

		}
	}

	namespace editor
	{

class IEditor;

class DefaultObjectEditor
:	public IObjectEditor
,	public ui::custom::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	DefaultObjectEditor(IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void apply() T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

	virtual ui::Size getPreferredSize() const T_OVERRIDE T_FINAL;

private:
	IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< ISerializable > m_object;
	Ref< ui::custom::AutoPropertyList > m_propertyList;

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const T_OVERRIDE T_FINAL;

	void eventPropertyCommand(ui::custom::PropertyCommandEvent* event);
};

	}
}

#endif	// traktor_editor_DefaultObjectEditor_H
