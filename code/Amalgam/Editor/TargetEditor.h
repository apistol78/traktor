#ifndef traktor_amalgam_TargetEditor_H
#define traktor_amalgam_TargetEditor_H

#include <list>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Editor/IObjectEditor.h"
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Container;
class DropDown;
class Edit;
class ListBox;

		namespace custom
		{

class EditList;
class EditListEditEvent;

		}
	}

	namespace amalgam
	{

class Feature;
class Platform;
class Target;
class TargetConfiguration;

class TargetEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	TargetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object);

	virtual void destroy();

	virtual void apply();

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

	virtual ui::Size getPreferredSize() const;

private:
	struct EditFeature : public Object
	{
		Ref< Feature > feature;
		Ref< db::Instance > featureInstance;

		bool operator < (const EditFeature& ef) const;
	};

	editor::IEditor* m_editor;
	Ref< db::Instance > m_editInstance;
	Ref< Target > m_editTarget;
	RefArray< db::Instance > m_platformInstances;
	std::list< EditFeature > m_features;
	Ref< ui::Container > m_containerOuter;
	Ref< ui::custom::EditList > m_listBoxTargetConfigurations;
	Ref< ui::DropDown > m_dropDownPlatform;
	Ref< ui::Edit > m_editExecutable;
	Ref< ui::ListBox > m_listBoxAvailFeatures;
	Ref< ui::ListBox > m_listBoxUsedFeatures;

	void updateTargetConfigurations();

	void updateAvailableFeatures();

	void updateUsedFeatures();

	void selectPlatform(const Guid& platformGuid) const;

	void eventListBoxTargetConfigurationsEdit(ui::custom::EditListEditEvent* event);

	void eventListBoxTargetConfigurationsSelect(ui::SelectionChangeEvent* event);

	void eventButtonNewTargetConfigurationClick(ui::ButtonClickEvent* event);

	void eventButtonCloneTargetConfigurationClick(ui::ButtonClickEvent* event);

	void eventButtonRemoveTargetConfigurationClick(ui::ButtonClickEvent* event);

	void eventDropDownPlatformSelect(ui::SelectionChangeEvent* event);

	void eventButtonAddFeatureClick(ui::ButtonClickEvent* event);

	void eventButtonRemoveFeatureClick(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_amalgam_TargetEditor_H
