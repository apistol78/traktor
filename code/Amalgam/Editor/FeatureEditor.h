/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_FeatureEditor_H
#define traktor_amalgam_FeatureEditor_H

#include "Amalgam/Editor/Deploy/Feature.h"
#include "Editor/IObjectEditor.h"

namespace traktor
{

class PropertyStringSet;

	namespace editor
	{
	
class IEditor;

	}

	namespace ui
	{
	
class Edit;
	
		namespace custom
		{
		
class EditList;
class ListBox;
		
		}
	}

	namespace amalgam
	{

class FeatureEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	FeatureEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void apply() T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

	virtual ui::Size getPreferredSize() const T_OVERRIDE T_FINAL;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< Feature > m_feature;
	Ref< ui::Edit > m_editName;
	Ref< ui::Edit > m_editPriority;
	Ref< ui::custom::ListBox > m_listPlatforms;
	Ref< ui::Edit > m_editExecutable;
	Ref< ui::custom::EditList > m_listKeys;
	Ref< ui::custom::EditList > m_listValues;

	Feature::Platform* m_selectedPlatform;
	std::wstring m_selectedKey;

	void selectPlatform(Feature::Platform* platform);

	void selectKey(const std::wstring& key);
};

	}
}

#endif	// traktor_amalgam_FeatureEditor_H
