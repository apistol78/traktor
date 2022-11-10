/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Runtime/Editor/FeatureEditor.h"
#include "Runtime/Editor/Deploy/Platform.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/ListBox/ListBox.h"
#include "Ui/EditList.h"
#include "Ui/EditListEditEvent.h"
#include "Ui/FlowLayout.h"
#include "Ui/Panel.h"
#include "Ui/Splitter.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.FeatureEditor", FeatureEditor, editor::IObjectEditor)

FeatureEditor::FeatureEditor(editor::IEditor* editor)
:	m_editor(editor)
,	m_selectedPlatform(nullptr)
{
}

bool FeatureEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	const int32_t f = ui::dpi96(4);

	m_instance = instance;
	m_feature = mandatory_non_null_type_cast< Feature* >(object);

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", f, f));

	Ref< ui::Panel > containerName = new ui::Panel();
	containerName->create(container, i18n::Text(L"RUNTIME_FEATURE"), new ui::TableLayout(L"*,100%", L"*", 2 * f, f));

	Ref< ui::Static > staticName = new ui::Static();
	staticName->create(containerName, i18n::Text(L"RUNTIME_FEATURE_DESCRIPTION"));

	m_editName = new ui::Edit();
	m_editName->create(containerName, m_feature->getDescription());
	m_editName->addEventHandler< ui::ContentChangeEvent >([&](ui::ContentChangeEvent* event) {
		m_feature->setDescription(m_editName->getText());
	});

	Ref< ui::Static > staticPriority = new ui::Static();
	staticPriority->create(containerName, i18n::Text(L"RUNTIME_FEATURE_PRIORITY"));

	m_editPriority = new ui::Edit();
	m_editPriority->create(containerName, toString(m_feature->getPriority()), ui::WsNone, new ui::NumericEditValidator(false));
	m_editPriority->addEventHandler< ui::ContentChangeEvent >([&](ui::ContentChangeEvent* event) {
		m_feature->setPriority(parseString< int32_t >(m_editPriority->getText()));
	});

	Ref< ui::Splitter > splitterInner = new ui::Splitter();
	splitterInner->create(container, true, ui::dpi96(200));

	Ref< ui::Container > containerPlatforms = new ui::Container();
	containerPlatforms->create(splitterInner, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, f));

	m_listPlatforms = new ui::ListBox();
	m_listPlatforms->create(containerPlatforms, ui::ListBox::WsSort);
	m_listPlatforms->addEventHandler< ui::SelectionChangeEvent >([&](ui::SelectionChangeEvent* event) {
		Ref< db::Instance > platformInstance = m_listPlatforms->getSelectedData< db::Instance >();
		Feature::Platform* platform = m_feature->getPlatform(platformInstance->getGuid());
		selectPlatform(platform);
	});

	for (auto platform : m_feature->getPlatforms())
	{
		Ref< db::Instance > platformInstance = m_editor->getSourceDatabase()->getInstance(platform.platform);
		if (!platformInstance)
			continue;

		m_listPlatforms->add(platformInstance->getName(), platformInstance);
	}

	Ref< ui::Container > containerPlatformsAddRemove = new ui::Container();
	containerPlatformsAddRemove->create(containerPlatforms, ui::WsNone, new ui::FlowLayout(0, 0, f, f));

	Ref< ui::Button > buttonAdd = new ui::Button();
	buttonAdd->create(containerPlatformsAddRemove, i18n::Text(L"RUNTIME_FEATURE_ADD"));
	buttonAdd->addEventHandler< ui::ButtonClickEvent >([&](ui::ButtonClickEvent* event) {
		Ref< db::Instance > platformInstance = m_editor->browseInstance(type_of< Platform >());
		if (platformInstance)
		{
			Feature::Platform fp;
			fp.platform = platformInstance->getGuid();
			fp.deploy = new PropertyGroup();
			m_feature->addPlatform(fp);
			m_listPlatforms->add(platformInstance->getName(), platformInstance);
		}
	});

	Ref< ui::Button > buttonClone = new ui::Button();
	buttonClone->create(containerPlatformsAddRemove, i18n::Text(L"RUNTIME_FEATURE_CLONE"));
	buttonClone->addEventHandler< ui::ButtonClickEvent >([&](ui::ButtonClickEvent* event) {
		if (!m_selectedPlatform)
			return;

		Ref< db::Instance > platformInstance = m_editor->browseInstance(type_of< Platform >());
		if (!platformInstance)
			return;

		Feature::Platform fp;
		fp.platform = platformInstance->getGuid();
		fp.deploy = DeepClone(m_selectedPlatform->deploy).create< PropertyGroup >();

		if (fp.deploy->getProperty(L"DEPLOY_SHARED_PATH_DEBUG") != nullptr)
			fp.deploy->setProperty< PropertyStringSet >(L"DEPLOY_SHARED_PATH_DEBUG", SmallSet< std::wstring >({ L"__path_shared_debug" }));
		if (fp.deploy->getProperty(L"DEPLOY_SHARED_PATH_RELEASE") != nullptr)
			fp.deploy->setProperty< PropertyStringSet >(L"DEPLOY_SHARED_PATH_RELEASE", SmallSet< std::wstring >({ L"__path_shared_release" }));
		if (fp.deploy->getProperty(L"DEPLOY_STATIC_PATH_DEBUG") != nullptr)
			fp.deploy->setProperty< PropertyStringSet >(L"DEPLOY_STATIC_PATH_DEBUG", SmallSet< std::wstring >({ L"__path_static_debug" }));
		if (fp.deploy->getProperty(L"DEPLOY_STATIC_PATH_RELEASE") != nullptr)
			fp.deploy->setProperty< PropertyStringSet >(L"DEPLOY_STATIC_PATH_RELEASE", SmallSet< std::wstring >({ L"__path_static_release" }));

		m_feature->addPlatform(fp);
		m_listPlatforms->add(platformInstance->getName(), platformInstance);
	});

	Ref< ui::Button > buttonRemove = new ui::Button();
	buttonRemove->create(containerPlatformsAddRemove, i18n::Text(L"RUNTIME_FEATURE_REMOVE"));
	buttonRemove->addEventHandler< ui::ButtonClickEvent >([&](ui::ButtonClickEvent* event) {
		int32_t index = m_listPlatforms->getSelected();
		if (index >= 0)
		{
			auto platformInstance = m_listPlatforms->getData< db::Instance >(index);
			T_ASSERT(platformInstance != nullptr);

			if (m_feature->removePlatform(platformInstance->getGuid()))
				m_listPlatforms->remove(index);
		}
	});

	Ref< ui::Panel > containerEdit = new ui::Panel();
	containerEdit->create(splitterInner, i18n::Text(L"RUNTIME_FEATURE_PLATFORM"), new ui::TableLayout(L"100%", L"*,100%", 2 * f, f));

	Ref< ui::Container > containerExecutable = new ui::Container();
	containerExecutable->create(containerEdit, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, f));

	Ref< ui::Static > staticExecutable = new ui::Static();
	staticExecutable->create(containerExecutable, i18n::Text(L"RUNTIME_FEATURE_EXECUTABLE"));

	m_editExecutable = new ui::Edit();
	m_editExecutable->create(containerExecutable, L"");
	m_editExecutable->addEventHandler< ui::ContentChangeEvent >([&](ui::ContentChangeEvent* event) {
		if (m_selectedPlatform)
			m_selectedPlatform->executableFile = m_editExecutable->getText();
	});

	Ref< ui::Splitter > splitterEnvironment = new ui::Splitter();
	splitterEnvironment->create(containerEdit, true, ui::dpi96(220));

	m_listKeys = new ui::EditList();
	m_listKeys->create(splitterEnvironment, ui::EditList::WsAutoAdd | ui::EditList::WsAutoRemove);
	m_listKeys->addEventHandler< ui::SelectionChangeEvent >([&](ui::SelectionChangeEvent* event) {
		selectKey(m_listKeys->getSelectedItem());
	});
	m_listKeys->addEventHandler< ui::EditListEditEvent >([&](ui::EditListEditEvent* event) {
		if (!m_selectedPlatform)
			return;

		auto deploy = m_selectedPlatform->deploy;

		// Create property group if no values exist.
		if (deploy == nullptr)
			deploy = m_selectedPlatform->deploy = new PropertyGroup();

		if (event->getIndex() < 0)	// Add key
		{
			if (deploy->getProperty(event->getText()) == nullptr)
			{
				deploy->setProperty(
					event->getText(),
					new PropertyStringSet()
				);
				event->consume();
			}
			else
				log::error << L"Key \"" << event->getText() << L"\" already exists." << Endl;
		}
		else if (event->getText().empty())	// Remove key
		{
			std::wstring fromName = m_listKeys->getItem(event->getIndex());
			deploy->setProperty(
				fromName,
				nullptr
			);
			event->consume();
		}
		else	// Rename key
		{
			std::wstring fromName = m_listKeys->getItem(event->getIndex());
			Ref< IPropertyValue > property = deploy->getProperty(fromName);
			deploy->setProperty(fromName, nullptr);
			deploy->setProperty(event->getText(), property);
			event->consume();
		}
	});

	m_listValues = new ui::EditList();
	m_listValues->create(splitterEnvironment, ui::EditList::WsAutoAdd | ui::EditList::WsAutoRemove);
	m_listValues->addEventHandler< ui::EditListEditEvent >([&](ui::EditListEditEvent* event) {
		if (!m_selectedPlatform)
			return;

		auto values = m_selectedPlatform->deploy->getProperty< SmallSet< std::wstring > >(m_selectedKey);

		if (event->getIndex() < 0)	// Add value
		{
			values.insert(event->getText());
			event->consume();
		}
		else if (event->getText().empty())	// Remove value
		{
			std::wstring fromValue = m_listValues->getItem(event->getIndex());
			values.erase(fromValue);
			event->consume();
		}
		else	// Rename value
		{
			std::wstring fromValue = m_listValues->getItem(event->getIndex());
			values.erase(fromValue);
			values.insert(event->getText());
			event->consume();
		}

		if (event->consumed())
			m_selectedPlatform->deploy->setProperty< PropertyStringSet >(m_selectedKey, values);
	});

	selectPlatform(nullptr);
	return true;
}

void FeatureEditor::destroy()
{
}

void FeatureEditor::apply()
{
	m_instance->setObject(m_feature);
}

bool FeatureEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void FeatureEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size FeatureEditor::getPreferredSize() const
{
	return ui::Size(
		ui::dpi96(900),
		ui::dpi96(600)
	);
}

void FeatureEditor::selectPlatform(Feature::Platform* platform)
{
	int32_t selected = -1;

	m_selectedPlatform = platform;

	m_listKeys->removeAll();
	if (m_selectedPlatform != nullptr)
	{
		m_editExecutable->setText(m_selectedPlatform->executableFile);
		m_editExecutable->setEnable(true);

		if (m_selectedPlatform->deploy != nullptr)
		{
			for (auto kv : m_selectedPlatform->deploy->getValues())
			{
				int32_t index = m_listKeys->add(kv.first, kv.second);
				if (kv.first == m_selectedKey)
					selected = index;
			}
			m_listKeys->select(selected);
		}
	}
	else
	{
		m_editExecutable->setText(L"");
		m_editExecutable->setEnable(false);
	}

	selectKey(selected >= 0 ? m_selectedKey : L"");
}

void FeatureEditor::selectKey(const std::wstring& key)
{
	m_selectedKey = key;

	m_listValues->removeAll();

	if (m_selectedPlatform != nullptr)
	{
		if (m_selectedPlatform->deploy != nullptr)
		{
			for (auto v : m_selectedPlatform->deploy->getProperty< SmallSet< std::wstring > >(key))
				m_listValues->add(v);
		}
		m_listValues->setEnable(true);
	}
	else
		m_listValues->setEnable(false);
}

	}
}
