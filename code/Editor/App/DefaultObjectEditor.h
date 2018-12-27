/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_DefaultObjectEditor_H
#define traktor_editor_DefaultObjectEditor_H

#include "Editor/IObjectEditor.h"
#include "Ui/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace ui
	{

class PropertyCommandEvent;

	}

	namespace editor
	{

class IEditor;

class DefaultObjectEditor
:	public IObjectEditor
,	public ui::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	DefaultObjectEditor(IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) override final;

	virtual void destroy() override final;

	virtual void apply() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual ui::Size getPreferredSize() const override final;

private:
	IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< ISerializable > m_object;
	Ref< ui::AutoPropertyList > m_propertyList;

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const override final;

	void eventPropertyCommand(ui::PropertyCommandEvent* event);
};

	}
}

#endif	// traktor_editor_DefaultObjectEditor_H
