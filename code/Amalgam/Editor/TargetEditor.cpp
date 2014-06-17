#include "Amalgam/Editor/Feature.h"
#include "Amalgam/Editor/Platform.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetConfiguration.h"
#include "Amalgam/Editor/TargetEditor.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "Ui/Button.h"
#include "Ui/Container.h"
#include "Ui/DropDown.h"
#include "Ui/Edit.h"
#include "Ui/FileDialog.h"
#include "Ui/FlowLayout.h"
#include "Ui/ListBox.h"
#include "Ui/MethodHandler.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/EditList.h"
#include "Ui/Custom/InputDialog.h"
#include "Ui/Custom/Panel.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Events/EditEvent.h"

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
	m_listBoxTargetConfigurations->addEditEventHandler(ui::createMethodHandler(this, &TargetEditor::eventListBoxTargetConfigurationsEdit));
	m_listBoxTargetConfigurations->addSelectEventHandler(ui::createMethodHandler(this, &TargetEditor::eventListBoxTargetConfigurationsSelect));

	Ref< ui::Container > containerManageTargetConfigurations = new ui::Container();
	containerManageTargetConfigurations->create(containerTargetConfigurations, ui::WsNone, new ui::FlowLayout(0, 0, 4, 4));

	Ref< ui::Button > buttonNewTargetConfiguration = new ui::Button();
	buttonNewTargetConfiguration->create(containerManageTargetConfigurations, L"New...");
	buttonNewTargetConfiguration->addClickEventHandler(ui::createMethodHandler(this, &TargetEditor::eventButtonNewTargetConfigurationClick));

	Ref< ui::Button > buttonCloneTargetConfiguration = new ui::Button();
	buttonCloneTargetConfiguration->create(containerManageTargetConfigurations, L"Clone");
	buttonCloneTargetConfiguration->addClickEventHandler(ui::createMethodHandler(this, &TargetEditor::eventButtonCloneTargetConfigurationClick));

	Ref< ui::Button > buttonRemoveTargetConfiguration = new ui::Button();
	buttonRemoveTargetConfiguration->create(containerManageTargetConfigurations, L"Delete");
	buttonRemoveTargetConfiguration->addClickEventHandler(ui::createMethodHandler(this, &TargetEditor::eventButtonRemoveTargetConfigurationClick));

	Ref< ui::Container > containerEditTargetConfiguration = new ui::Container();
	containerEditTargetConfiguration->create(splitterInner, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 4));

	Ref< ui::custom::Panel > panelGeneral = new ui::custom::Panel();
	panelGeneral->create(containerEditTargetConfiguration, L"General", new ui::TableLayout(L"*,100%,*,100%", L"*,*", 8, 4));

	Ref< ui::Static > staticPlatform = new ui::Static();
	staticPlatform->create(panelGeneral, L"Platform");

	m_dropDownPlatform = new ui::DropDown();
	m_dropDownPlatform->create(panelGeneral);
	m_dropDownPlatform->addSelectEventHandler(ui::createMethodHandler(this, &TargetEditor::eventDropDownPlatformSelect));

	Ref< ui::Static > staticBuildRoot = new ui::Static();
	staticBuildRoot->create(panelGeneral, L"Build root");

	Ref< ui::Static > staticBuildRootInstance = new ui::Static();
	staticBuildRootInstance->create(panelGeneral, L"{0000000-000000-000000-000000}");

	Ref< ui::Static > staticExecutable = new ui::Static();
	staticExecutable->create(panelGeneral, L"Executable");

	m_editExecutable = new ui::Edit();
	m_editExecutable->create(panelGeneral);

	Ref< ui::Static > staticStartup = new ui::Static();
	staticStartup->create(panelGeneral, L"Startup");

	Ref< ui::Static > staticStartupInstance = new ui::Static();
	staticStartupInstance->create(panelGeneral, L"{0000000-000000-000000-000000}");

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
	buttonAddFeature->addClickEventHandler(ui::createMethodHandler(this, &TargetEditor::eventButtonAddFeatureClick));

	Ref< ui::Button > buttonRemoveFeature = new ui::Button();
	buttonRemoveFeature->create(containerManageFeatures, L"<");
	buttonRemoveFeature->addClickEventHandler(ui::createMethodHandler(this, &TargetEditor::eventButtonRemoveFeatureClick));

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
			if (targetConfiguration->haveFeature(i->featureInstance->getGuid()))
				continue;

			m_listBoxAvailFeatures->add(i->feature->getDescription(), i->featureInstance);
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

void TargetEditor::eventListBoxTargetConfigurationsEdit(ui::Event* event)
{
	ui::EditEvent* editEvent = checked_type_cast< ui::EditEvent*, false >(event);

	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		targetConfiguration->setName(editEvent->getText());
		editEvent->consume();
	}
}

void TargetEditor::eventListBoxTargetConfigurationsSelect(ui::Event* event)
{
	updateAvailableFeatures();
	updateUsedFeatures();

	m_editExecutable->setText(L"");
	m_dropDownPlatform->select(-1);

	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		m_editExecutable->setText(targetConfiguration->getExecutable());

		Guid platformGuid = targetConfiguration->getPlatform();
		selectPlatform(platformGuid);
	}
}

void TargetEditor::eventButtonNewTargetConfigurationClick(ui::Event* event)
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

		m_listBoxTargetConfigurations->select(-1);
	}
}

void TargetEditor::eventButtonCloneTargetConfigurationClick(ui::Event* event)
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

		m_listBoxTargetConfigurations->select(-1);
	}
}

void TargetEditor::eventButtonRemoveTargetConfigurationClick(ui::Event* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	m_editTarget->removeConfiguration(targetConfiguration);

	updateTargetConfigurations();
	updateAvailableFeatures();
	updateUsedFeatures();

	m_listBoxTargetConfigurations->select(-1);
}

void TargetEditor::eventDropDownPlatformSelect(ui::Event* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	db::Instance* platformInstance = m_dropDownPlatform->getSelectedData< db::Instance >();
	T_ASSERT (platformInstance);

	targetConfiguration->setPlatform(platformInstance->getGuid());
}

void TargetEditor::eventButtonAddFeatureClick(ui::Event* event)
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

void TargetEditor::eventButtonRemoveFeatureClick(ui::Event* event)
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
