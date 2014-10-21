#include "Amalgam/Editor/Feature.h"
#include "Amalgam/Editor/Platform.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetConfiguration.h"
#include "Amalgam/Editor/TargetEditor.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Editor/IEditor.h"
#include "Ui/Bitmap.h"
#include "Ui/Button.h"
#include "Ui/Container.h"
#include "Ui/DropDown.h"
#include "Ui/Edit.h"
#include "Ui/FileDialog.h"
#include "Ui/FlowLayout.h"
#include "Ui/Image.h"
#include "Ui/ListBox.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/EditList.h"
#include "Ui/Custom/EditListEditEvent.h"
#include "Ui/Custom/InputDialog.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Custom/Panel.h"
#include "Ui/Custom/Splitter.h"

// Resources
#include "Resources/NoIcon.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetEditor", TargetEditor, editor::IObjectEditor)

TargetEditor::TargetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool TargetEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_editInstance = instance;
	m_editTarget = checked_type_cast< Target* >(object);

	m_containerOuter = new ui::Container();
	m_containerOuter->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 4, 4));

	Ref< ui::custom::Splitter > splitterInner = new ui::custom::Splitter();
	splitterInner->create(m_containerOuter, true, 200, false, 100);

	Ref< ui::Container > containerTargetConfigurations = new ui::Container();
	containerTargetConfigurations->create(splitterInner, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, 4));

	m_listBoxTargetConfigurations = new ui::custom::EditList();
	m_listBoxTargetConfigurations->create(containerTargetConfigurations, ui::ListBox::WsDefault);
	m_listBoxTargetConfigurations->addEventHandler< ui::custom::EditListEditEvent >(this, &TargetEditor::eventListBoxTargetConfigurationsEdit);
	m_listBoxTargetConfigurations->addEventHandler< ui::SelectionChangeEvent >(this, &TargetEditor::eventListBoxTargetConfigurationsSelect);

	Ref< ui::Container > containerManageTargetConfigurations = new ui::Container();
	containerManageTargetConfigurations->create(containerTargetConfigurations, ui::WsNone, new ui::FlowLayout(0, 0, 4, 4));

	Ref< ui::Button > buttonNewTargetConfiguration = new ui::Button();
	buttonNewTargetConfiguration->create(containerManageTargetConfigurations, L"New...");
	buttonNewTargetConfiguration->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventButtonNewTargetConfigurationClick);

	Ref< ui::Button > buttonCloneTargetConfiguration = new ui::Button();
	buttonCloneTargetConfiguration->create(containerManageTargetConfigurations, L"Clone");
	buttonCloneTargetConfiguration->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventButtonCloneTargetConfigurationClick);

	Ref< ui::Button > buttonRemoveTargetConfiguration = new ui::Button();
	buttonRemoveTargetConfiguration->create(containerManageTargetConfigurations, L"Delete");
	buttonRemoveTargetConfiguration->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventButtonRemoveTargetConfigurationClick);

	Ref< ui::Container > containerEditTargetConfiguration = new ui::Container();
	containerEditTargetConfiguration->create(splitterInner, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 4));

	Ref< ui::custom::Panel > panelGeneral = new ui::custom::Panel();
	panelGeneral->create(containerEditTargetConfiguration, L"General", new ui::TableLayout(L"100%,128", L"*", 8, 4));

	Ref< ui::Container > containerLeft = new ui::Container();
	containerLeft->create(panelGeneral, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, 4));

	Ref< ui::Static > staticPlatform = new ui::Static();
	staticPlatform->create(containerLeft, L"Platform");

	m_dropDownPlatform = new ui::DropDown();
	m_dropDownPlatform->create(containerLeft);
	m_dropDownPlatform->addEventHandler< ui::SelectionChangeEvent >(this, &TargetEditor::eventDropDownPlatformSelect);

	Ref< ui::Static > staticBuildRoot = new ui::Static();
	staticBuildRoot->create(containerLeft, L"Build root");

	Ref< ui::Container > container1 = new ui::Container();
	container1->create(containerLeft, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, 4));

	m_editBuildRootInstance = new ui::Edit();
	m_editBuildRootInstance->create(container1, L"", ui::WsClientBorder | ui::Edit::WsReadOnly);
	m_editBuildRootInstance->setText(L"");

	Ref< ui::custom::MiniButton > buttonBuildRootInstance = new ui::custom::MiniButton();
	buttonBuildRootInstance->create(container1, L"...");
	buttonBuildRootInstance->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventBrowseRootButtonClick);

	Ref< ui::Static > staticStartup = new ui::Static();
	staticStartup->create(containerLeft, L"Startup");

	Ref< ui::Container > container2 = new ui::Container();
	container2->create(containerLeft, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, 4));

	m_editStartupInstance = new ui::Edit();
	m_editStartupInstance->create(container2, L"", ui::WsClientBorder | ui::Edit::WsReadOnly);
	m_editStartupInstance->setText(L"");

	Ref< ui::custom::MiniButton > buttonStartupInstance = new ui::custom::MiniButton();
	buttonStartupInstance->create(container2, L"...");
	buttonStartupInstance->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventBrowseStartupButtonClick);

	Ref< ui::Static > staticDefaultInput = new ui::Static();
	staticDefaultInput->create(containerLeft, L"Default input");

	Ref< ui::Container > container3 = new ui::Container();
	container3->create(containerLeft, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, 4));

	m_editDefaultInputInstance = new ui::Edit();
	m_editDefaultInputInstance->create(container3, L"", ui::WsClientBorder | ui::Edit::WsReadOnly);
	m_editDefaultInputInstance->setText(L"");

	Ref< ui::custom::MiniButton > buttonDefaultInputInstance = new ui::custom::MiniButton();
	buttonDefaultInputInstance->create(container3, L"...");
	buttonDefaultInputInstance->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventBrowseDefaultInputButtonClick);

	Ref< ui::Static > staticOnlineConfig = new ui::Static();
	staticOnlineConfig->create(containerLeft, L"Online configuration");

	Ref< ui::Container > container4 = new ui::Container();
	container4->create(containerLeft, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, 4));

	m_editOnlineConfigInstance = new ui::Edit();
	m_editOnlineConfigInstance->create(container4, L"", ui::WsClientBorder | ui::Edit::WsReadOnly);
	m_editOnlineConfigInstance->setText(L"");

	Ref< ui::custom::MiniButton > buttonOnlineConfigInstance = new ui::custom::MiniButton();
	buttonOnlineConfigInstance->create(container4, L"...");
	buttonOnlineConfigInstance->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventBrowseOnlineConfigButtonClick);

	m_bitmapNoIcon = ui::Bitmap::load(c_ResourceNoIcon, sizeof(c_ResourceNoIcon), L"png");
	
	m_imageIcon = new ui::Image();
	m_imageIcon->create(panelGeneral, m_bitmapNoIcon, ui::Image::WsTransparent | ui::WsDoubleBuffer);
	m_imageIcon->addEventHandler< ui::MouseButtonDownEvent >(this, &TargetEditor::eventBrowseIconClick);

	Ref< ui::custom::Panel > panelFeatures = new ui::custom::Panel();
	panelFeatures->create(containerEditTargetConfiguration, L"Features", new ui::TableLayout(L"100%,*,100%", L"100%", 8, 4));

	Ref< ui::Container > containerAvailFeatures = new ui::Container();
	containerAvailFeatures->create(panelFeatures, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 4));

	Ref< ui::Static > staticAvailFeatures = new ui::Static();
	staticAvailFeatures->create(containerAvailFeatures, L"Available");

	m_listBoxAvailFeatures = new ui::ListBox();
	m_listBoxAvailFeatures->create(containerAvailFeatures, L"", ui::WsClientBorder | ui::ListBox::WsMultiple);

	Ref< ui::Container > containerManageFeatures = new ui::Container();
	containerManageFeatures->create(panelFeatures, ui::WsNone, new ui::TableLayout(L"*", L"*,*", 0, 0));

	Ref< ui::Button > buttonAddFeature = new ui::Button();
	buttonAddFeature->create(containerManageFeatures, L">");
	buttonAddFeature->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventButtonAddFeatureClick);

	Ref< ui::Button > buttonRemoveFeature = new ui::Button();
	buttonRemoveFeature->create(containerManageFeatures, L"<");
	buttonRemoveFeature->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventButtonRemoveFeatureClick);

	Ref< ui::Container > containerUsedFeatures = new ui::Container();
	containerUsedFeatures->create(panelFeatures, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 4));

	Ref< ui::Static > staticUsedFeatures = new ui::Static();
	staticUsedFeatures->create(containerUsedFeatures, L"Using");

	m_listBoxUsedFeatures = new ui::ListBox();
	m_listBoxUsedFeatures->create(containerUsedFeatures, L"", ui::WsClientBorder | ui::ListBox::WsMultiple);

	// Collect all available platforms.
	db::recursiveFindChildInstances(
		m_editor->getSourceDatabase()->getRootGroup(),
		db::FindInstanceByType(type_of< Platform >()),
		m_platformInstances
	);

	// Collect all available features.
	RefArray< db::Instance > featureInstances;
	db::recursiveFindChildInstances(
		m_editor->getSourceDatabase()->getRootGroup(),
		db::FindInstanceByType(type_of< Feature >()),
		featureInstances
	);

	for (RefArray< db::Instance >::const_iterator i = featureInstances.begin(); i != featureInstances.end(); ++i)
	{
		if (*i)
		{
			EditFeature ef;
			ef.feature = (*i)->getObject< Feature >();
			ef.featureInstance = (*i);
			if (ef.feature)
				m_features.push_back(ef);
			else
				log::error << L"Unable to read feature \"" << (*i)->getName() << L"\"" << Endl;
		}
	}

	m_features.sort();

	// Add all entries of platform drop down.
	for (RefArray< db::Instance >::const_iterator i = m_platformInstances.begin(); i != m_platformInstances.end(); ++i)
	{
		m_dropDownPlatform->add(
			(*i)->getName(),
			*i
		);
	}
	
	updateTargetConfigurations();
	updateAvailableFeatures();
	updateUsedFeatures();

	return true;
}

void TargetEditor::destroy()
{
}

void TargetEditor::apply()
{
	m_editInstance->setObject(m_editTarget);
}

bool TargetEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void TargetEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size TargetEditor::getPreferredSize() const
{
	return ui::Size(1000, 600);
}

void TargetEditor::updateTargetConfigurations()
{
	m_listBoxTargetConfigurations->removeAll();

	int32_t selected = m_listBoxTargetConfigurations->getSelected();

	const RefArray< TargetConfiguration >& configurations = m_editTarget->getConfigurations();
	for (RefArray< TargetConfiguration >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		m_listBoxTargetConfigurations->add(
			(*i)->getName(),
			*i
		);
	}

	m_listBoxTargetConfigurations->select(selected);
}

void TargetEditor::updateAvailableFeatures()
{
	m_listBoxAvailFeatures->removeAll();

	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		for (std::list< EditFeature >::const_iterator i = m_features.begin(); i != m_features.end(); ++i)
		{
			if (i->feature != 0 && i->feature->getPlatform(targetConfiguration->getPlatform()) != 0)
			{
				if (targetConfiguration->haveFeature(i->featureInstance->getGuid()))
					continue;

				m_listBoxAvailFeatures->add(i->feature->getDescription(), i->featureInstance);
			}
		}
	}
}

void TargetEditor::updateUsedFeatures()
{
	m_listBoxUsedFeatures->removeAll();

	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		std::list< EditFeature > features;

		const std::list< Guid >& featureGuids = targetConfiguration->getFeatures();
		for (std::list< Guid >::const_iterator i = featureGuids.begin(); i != featureGuids.end(); ++i)
		{
			for (std::list< EditFeature >::const_iterator j = m_features.begin(); j != m_features.end(); ++j)
			{
				if (*i == j->featureInstance->getGuid())
				{
					features.push_back(*j);
					break;
				}
			}
		}

		features.sort();

		for (std::list< EditFeature >::const_iterator i = features.begin(); i != features.end(); ++i)
			m_listBoxUsedFeatures->add(i->feature->getDescription(), i->featureInstance);
	}
}

void TargetEditor::updateRoots()
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		Ref< db::Instance > rootInstance = m_editor->getSourceDatabase()->getInstance(targetConfiguration->getRoot());
		if (rootInstance)
			m_editBuildRootInstance->setText(rootInstance->getPath());
		else
			m_editBuildRootInstance->setText(targetConfiguration->getRoot().format());

		Ref< db::Instance > startupInstance = m_editor->getSourceDatabase()->getInstance(targetConfiguration->getStartup());
		if (startupInstance)
			m_editStartupInstance->setText(startupInstance->getPath());
		else
			m_editStartupInstance->setText(targetConfiguration->getStartup().format());

		Ref< db::Instance > defaultInputInstance = m_editor->getSourceDatabase()->getInstance(targetConfiguration->getDefaultInput());
		if (defaultInputInstance)
			m_editDefaultInputInstance->setText(defaultInputInstance->getPath());
		else
			m_editDefaultInputInstance->setText(targetConfiguration->getDefaultInput().format());

		Ref< db::Instance > onlineConfigInstance = m_editor->getSourceDatabase()->getInstance(targetConfiguration->getOnlineConfig());
		if (onlineConfigInstance)
			m_editOnlineConfigInstance->setText(onlineConfigInstance->getPath());
		else
			m_editOnlineConfigInstance->setText(targetConfiguration->getOnlineConfig().format());
	}
	else
	{
		m_editBuildRootInstance->setText(L"");
		m_editStartupInstance->setText(L"");
		m_editDefaultInputInstance->setText(L"");
		m_editOnlineConfigInstance->setText(L"");
	}
}

void TargetEditor::updateIcon()
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		Path systemRoot = m_editor->getWorkspaceSettings()->getProperty< PropertyString >(L"Amalgam.SystemRoot", L"$(TRAKTOR_HOME)");
		Path iconPath = targetConfiguration->getIcon();

		Ref< drawing::Image > iconImage = drawing::Image::load(systemRoot + iconPath);
		if (iconImage)
		{
			drawing::ScaleFilter scaleFilter(128, 128, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
			iconImage->apply(&scaleFilter);

			m_imageIcon->setImage(new ui::Bitmap(iconImage), true);
		}
		else
			m_imageIcon->setImage(m_bitmapNoIcon, true);
	}
	else
		m_imageIcon->setImage(m_bitmapNoIcon, true);
}

void TargetEditor::selectPlatform(const Guid& platformGuid) const
{
	int c = m_dropDownPlatform->count();
	for (int i = 0; i < c; ++i)
	{
		db::Instance* platformInstance = m_dropDownPlatform->getData< db::Instance >(i);
		T_ASSERT (platformInstance);

		if (platformInstance->getGuid() == platformGuid)
		{
			m_dropDownPlatform->select(i);
			return;
		}
	}
	m_dropDownPlatform->select(-1);
}

void TargetEditor::eventListBoxTargetConfigurationsEdit(ui::custom::EditListEditEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		targetConfiguration->setName(event->getText());
		event->consume();
	}
}

void TargetEditor::eventListBoxTargetConfigurationsSelect(ui::SelectionChangeEvent* event)
{
	updateAvailableFeatures();
	updateUsedFeatures();
	updateRoots();
	updateIcon();

	m_dropDownPlatform->select(-1);

	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		Guid platformGuid = targetConfiguration->getPlatform();
		selectPlatform(platformGuid);
	}
}

void TargetEditor::eventButtonNewTargetConfigurationClick(ui::ButtonClickEvent* event)
{
	ui::custom::InputDialog::Field fields[] =
	{
		{ L"Name", L"", 0, 0 }
	};

	Ref< ui::custom::InputDialog > dialogInputName = new ui::custom::InputDialog();
	dialogInputName->create(m_containerOuter, L"Enter name", L"Enter configuration name", fields, sizeof_array(fields));
	if (dialogInputName->showModal() == ui::DrOk)
	{
		Ref< TargetConfiguration > targetConfiguration = new TargetConfiguration();
		targetConfiguration->setName(fields[0].value);

		m_editTarget->addConfiguration(targetConfiguration);

		updateTargetConfigurations();
		updateAvailableFeatures();
		updateUsedFeatures();
		updateRoots();
		updateIcon();

		m_listBoxTargetConfigurations->select(-1);
	}
}

void TargetEditor::eventButtonCloneTargetConfigurationClick(ui::ButtonClickEvent* event)
{
	Ref< TargetConfiguration > targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	targetConfiguration = DeepClone(targetConfiguration).create< TargetConfiguration >();
	T_ASSERT (targetConfiguration);

	ui::custom::InputDialog::Field fields[] =
	{
		{ L"Name", targetConfiguration->getName(), 0, 0 }
	};

	Ref< ui::custom::InputDialog > dialogInputName = new ui::custom::InputDialog();
	dialogInputName->create(m_containerOuter, L"Enter name", L"Enter configuration name", fields, sizeof_array(fields));
	if (dialogInputName->showModal() == ui::DrOk)
	{
		targetConfiguration->setName(fields[0].value);

		m_editTarget->addConfiguration(targetConfiguration);

		updateTargetConfigurations();
		updateAvailableFeatures();
		updateUsedFeatures();
		updateRoots();
		updateIcon();

		m_listBoxTargetConfigurations->select(-1);
	}
}

void TargetEditor::eventButtonRemoveTargetConfigurationClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	m_editTarget->removeConfiguration(targetConfiguration);

	updateTargetConfigurations();
	updateAvailableFeatures();
	updateUsedFeatures();
	updateRoots();
	updateIcon();

	m_listBoxTargetConfigurations->select(-1);
}

void TargetEditor::eventDropDownPlatformSelect(ui::SelectionChangeEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	db::Instance* platformInstance = m_dropDownPlatform->getSelectedData< db::Instance >();
	T_ASSERT (platformInstance);

	targetConfiguration->setPlatform(platformInstance->getGuid());
}

void TargetEditor::eventBrowseRootButtonClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	Ref< db::Instance > rootInstance = m_editor->browseInstance();
	if (rootInstance)
		targetConfiguration->setRoot(rootInstance->getGuid());

	updateRoots();
}

void TargetEditor::eventBrowseStartupButtonClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	Ref< db::Instance > startupInstance = m_editor->browseInstance();
	if (startupInstance)
		targetConfiguration->setStartup(startupInstance->getGuid());

	updateRoots();
}

void TargetEditor::eventBrowseDefaultInputButtonClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	Ref< db::Instance > defaultInputInstance = m_editor->browseInstance();
	if (defaultInputInstance)
		targetConfiguration->setDefaultInput(defaultInputInstance->getGuid());

	updateRoots();
}

void TargetEditor::eventBrowseOnlineConfigButtonClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	Ref< db::Instance > onlineConfigInstance = m_editor->browseInstance();
	if (onlineConfigInstance)
		targetConfiguration->setOnlineConfig(onlineConfigInstance->getGuid());

	updateRoots();
}

void TargetEditor::eventBrowseIconClick(ui::MouseButtonDownEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	ui::FileDialog fileDialog;
	if (fileDialog.create(m_containerOuter, L"Select icon image", L"All files;*.*"))
	{
		Path fileName;
		if (fileDialog.showModal(fileName) == ui::DrOk)
		{
			std::wstring systemRoot = m_editor->getWorkspaceSettings()->getProperty< PropertyString >(L"Amalgam.SystemRoot", L"$(TRAKTOR_HOME)");

			Path relativePath;
			if (!FileSystem::getInstance().getRelativePath(fileName, systemRoot, relativePath))
				relativePath = fileName;

			targetConfiguration->setIcon(relativePath.getPathName());
			updateIcon();
		}
		fileDialog.destroy();
	}
}

void TargetEditor::eventButtonAddFeatureClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	std::vector< int32_t > s;
	m_listBoxAvailFeatures->getSelected(s);

	for (std::vector< int32_t >::const_iterator i = s.begin(); i != s.end(); ++i)
	{
		Ref< db::Instance > featureInstance = m_listBoxAvailFeatures->getData< db::Instance >(*i);
		if (featureInstance)
			targetConfiguration->addFeature(featureInstance->getGuid());
	}

	updateAvailableFeatures();
	updateUsedFeatures();
}

void TargetEditor::eventButtonRemoveFeatureClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	std::vector< int32_t > s;
	m_listBoxUsedFeatures->getSelected(s);

	for (std::vector< int32_t >::const_iterator i = s.begin(); i != s.end(); ++i)
	{
		Ref< db::Instance > featureInstance = m_listBoxUsedFeatures->getData< db::Instance >(*i);
		if (featureInstance)
			targetConfiguration->removeFeature(featureInstance->getGuid());
	}

	updateAvailableFeatures();
	updateUsedFeatures();
}

bool TargetEditor::EditFeature::operator < (const EditFeature& ef) const
{
	return compareIgnoreCase(feature->getDescription(), ef.feature->getDescription()) < 0;
}

	}
}
