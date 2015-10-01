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

class Bitmap;
class Container;
class DropDown;
class Edit;
class Image;
class ListBox;

		namespace custom
		{

class EditList;
class EditListEditEvent;
class MiniButton;

		}
	}

	namespace amalgam
	{

class Feature;
class Platform;
class Target;
class TargetConfiguration;

/*! \brief
 * \ingroup Amalgam
 */
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
	Ref< ui::Bitmap > m_bitmapNoIcon;
	Ref< ui::Container > m_containerOuter;
	Ref< ui::custom::EditList > m_listBoxTargetConfigurations;
	Ref< ui::Image > m_imageIcon;
	Ref< ui::DropDown > m_dropDownPlatform;
	Ref< ui::custom::MiniButton > m_buttonBuildRootInstance;
	Ref< ui::custom::MiniButton > m_buttonStartupInstance;
	Ref< ui::custom::MiniButton > m_buttonDefaultInputInstance;
	Ref< ui::custom::MiniButton > m_buttonOnlineConfigInstance;
	Ref< ui::Edit > m_editBuildRootInstance;
	Ref< ui::Edit > m_editStartupInstance;
	Ref< ui::Edit > m_editDefaultInputInstance;
	Ref< ui::Edit > m_editOnlineConfigInstance;
	Ref< ui::ListBox > m_listBoxAvailFeatures;
	Ref< ui::ListBox > m_listBoxUsedFeatures;

	void updateTargetConfigurations();

	void updateAvailableFeatures();

	void updateUsedFeatures();

	void updateRoots();

	void updateIcon();

	void selectPlatform(const Guid& platformGuid) const;

	void eventListBoxTargetConfigurationsEdit(ui::custom::EditListEditEvent* event);

	void eventListBoxTargetConfigurationsSelect(ui::SelectionChangeEvent* event);

	void eventButtonNewTargetConfigurationClick(ui::ButtonClickEvent* event);

	void eventButtonCloneTargetConfigurationClick(ui::ButtonClickEvent* event);

	void eventButtonRemoveTargetConfigurationClick(ui::ButtonClickEvent* event);

	void eventDropDownPlatformSelect(ui::SelectionChangeEvent* event);

	void eventBrowseRootButtonClick(ui::ButtonClickEvent* event);

	void eventBrowseStartupButtonClick(ui::ButtonClickEvent* event);

	void eventBrowseDefaultInputButtonClick(ui::ButtonClickEvent* event);

	void eventBrowseOnlineConfigButtonClick(ui::ButtonClickEvent* event);

	void eventBrowseIconClick(ui::MouseButtonDownEvent* event);

	void eventButtonAddFeatureClick(ui::ButtonClickEvent* event);

	void eventButtonRemoveFeatureClick(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_amalgam_TargetEditor_H
