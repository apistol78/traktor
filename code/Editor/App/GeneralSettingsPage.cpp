#include "Editor/App/GeneralSettingsPage.h"
#include "Editor/Settings.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/DropDown.h"
#include "Ui/CheckBox.h"
#include "Ui/Static.h"
#include "Ui/Edit.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.GeneralSettingsPage", 0, GeneralSettingsPage, ISettingsPage)

bool GeneralSettingsPage::create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 0, 4)))
		return false;

	Ref< ui::Container > containerInner = new ui::Container();
	if (!containerInner->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, 4)))
		return false;

	Ref< ui::Static > staticSourceDatabase = new ui::Static();
	staticSourceDatabase->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_SOURCE_DATABASE"));

	m_editSourceDatabase = new ui::Edit();
	m_editSourceDatabase->create(containerInner, settings->getProperty< PropertyString >(L"Editor.SourceDatabase", L""));

	Ref< ui::Static > staticOutputDatabase = new ui::Static();
	staticOutputDatabase->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_OUTPUT_DATABASE"));

	m_editOutputDatabase = new ui::Edit();
	m_editOutputDatabase->create(containerInner, settings->getProperty< PropertyString >(L"Editor.OutputDatabase", L""));

	Ref< ui::Static > staticRenderer = new ui::Static();
	staticRenderer->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_RENDERER"));

	m_dropRenderSystem = new ui::DropDown();
	m_dropRenderSystem->create(containerInner, L"");

	std::wstring renderSystemType = settings->getProperty< PropertyString >(L"Editor.RenderSystem");

	std::vector< const TypeInfo* > renderSystemTypes;
	type_of< render::IRenderSystem >().findAllOf(renderSystemTypes, false);

	for (std::vector< const TypeInfo* >::const_iterator i = renderSystemTypes.begin(); i != renderSystemTypes.end(); ++i)
	{
		std::wstring name = (*i)->getName();

		int index = m_dropRenderSystem->add(name);
		if (name == renderSystemType)
			m_dropRenderSystem->select(index);
	}

	Ref< ui::Static > staticAssetPath = new ui::Static();
	staticAssetPath->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_ASSET_PATH"));

	m_editAssetPath = new ui::Edit();
	m_editAssetPath->create(containerInner, settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L""));

	Ref< ui::Static > staticDictionary = new ui::Static();
	staticDictionary->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_DICTIONARY"));

	m_editDictionary = new ui::Edit();
	m_editDictionary->create(containerInner, settings->getProperty< PropertyString >(L"Editor.Dictionary"));

	m_checkAutoSave = new ui::CheckBox();
	m_checkAutoSave->create(container, i18n::Text(L"EDITOR_SETTINGS_AUTOSAVE"));
	m_checkAutoSave->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.AutoSave"));

	m_checkBuildWhenSourceModified = new ui::CheckBox();
	m_checkBuildWhenSourceModified->create(container, i18n::Text(L"EDITOR_SETTINGS_BUILD_WHEN_SOURCE_MODIFIED"));
	m_checkBuildWhenSourceModified->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.BuildWhenSourceModified"));

	m_checkBuildWhenAssetModified = new ui::CheckBox();
	m_checkBuildWhenAssetModified->create(container, i18n::Text(L"EDITOR_SETTINGS_BUILD_WHEN_ASSET_MODIFIED"));
	m_checkBuildWhenAssetModified->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.BuildWhenAssetModified"));

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_GENERAL"));
	return true;
}

void GeneralSettingsPage::destroy()
{
}

bool GeneralSettingsPage::apply(Settings* settings)
{
	settings->setProperty< PropertyString >(L"Editor.RenderSystem", m_dropRenderSystem->getSelectedItem());
	settings->setProperty< PropertyString >(L"Editor.Dictionary", m_editDictionary->getText());
	settings->setProperty< PropertyString >(L"Pipeline.AssetPath", m_editAssetPath->getText());
	settings->setProperty< PropertyBoolean >(L"Editor.AutoSave", m_checkAutoSave->isChecked());
	settings->setProperty< PropertyBoolean >(L"Editor.BuildWhenSourceModified", m_checkBuildWhenSourceModified->isChecked());
	settings->setProperty< PropertyBoolean >(L"Editor.BuildWhenAssetModified", m_checkBuildWhenAssetModified->isChecked());
	return true;
}

	}
}
