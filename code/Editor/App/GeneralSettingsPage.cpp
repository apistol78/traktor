#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/App/GeneralSettingsPage.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/DropDown.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Itf/IWidgetFactory.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.GeneralSettingsPage", 0, GeneralSettingsPage, ISettingsPage)

bool GeneralSettingsPage::create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 0, ui::scaleBySystemDPI(4))))
		return false;

	Ref< ui::Container > containerInner = new ui::Container();
	if (!containerInner->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, ui::scaleBySystemDPI(4))))
		return false;

	Ref< ui::Static > staticDictionary = new ui::Static();
	staticDictionary->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_DICTIONARY"));

	m_editDictionary = new ui::Edit();
	m_editDictionary->create(containerInner, settings->getProperty< PropertyString >(L"Editor.Dictionary", L"$(TRAKTOR_HOME)/res/English.xml"));

	Ref< ui::Static > staticStyleSheet = new ui::Static();
	staticStyleSheet->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_STYLESHEET"));

	m_editStyleSheet = new ui::Edit();
	m_editStyleSheet->create(containerInner, settings->getProperty< PropertyString >(L"Editor.StyleSheet", L"$(TRAKTOR_HOME)/res/themes/Light/StyleSheet.xss"));

	Ref< ui::Static > staticFont = new ui::Static();
	staticFont->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_EDITOR_FONT"));

	Ref< ui::Container > containerFont = new ui::Container();
	if (!containerFont->create(containerInner, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, ui::scaleBySystemDPI(4))))
		return false;

	m_dropFonts = new ui::DropDown();
	m_dropFonts->create(containerFont);

	std::list< std::wstring > fonts;
	ui::Application::getInstance()->getWidgetFactory()->getSystemFonts(fonts);
	for (std::list< std::wstring >::const_iterator i = fonts.begin(); i != fonts.end(); ++i)
		m_dropFonts->add(*i);

	m_dropFonts->select(settings->getProperty< PropertyString >(L"Editor.Font", L"Consolas"));

	m_editFontSize = new ui::Edit();
	m_editFontSize->create(containerFont, L"1", ui::WsClientBorder, new ui::NumericEditValidator(false, 1, 100, 0));
	m_editFontSize->setText(toString(settings->getProperty< PropertyInteger >(L"Editor.FontSize", 14)));

	m_checkAutoOpen = new ui::CheckBox();
	m_checkAutoOpen->create(container, i18n::Text(L"EDITOR_SETTINGS_AUTOOPEN_RECENT_WORKSPACE"));
	m_checkAutoOpen->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.AutoOpenRecentlyUsedWorkspace"));

	m_checkAutoSave = new ui::CheckBox();
	m_checkAutoSave->create(container, i18n::Text(L"EDITOR_SETTINGS_AUTOSAVE"));
	m_checkAutoSave->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.AutoSave"));

	m_checkBuildWhenSourceModified = new ui::CheckBox();
	m_checkBuildWhenSourceModified->create(container, i18n::Text(L"EDITOR_SETTINGS_BUILD_WHEN_SOURCE_MODIFIED"));
	m_checkBuildWhenSourceModified->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.BuildWhenSourceModified"));

	m_checkBuildWhenAssetModified = new ui::CheckBox();
	m_checkBuildWhenAssetModified->create(container, i18n::Text(L"EDITOR_SETTINGS_BUILD_WHEN_ASSET_MODIFIED"));
	m_checkBuildWhenAssetModified->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.BuildWhenAssetModified"));

	m_checkBuildAfterBrowseInstance = new ui::CheckBox();
	m_checkBuildAfterBrowseInstance->create(container, i18n::Text(L"EDITOR_SETTINGS_BUILD_AFTER_BROWSE_INSTANCE"));
	m_checkBuildAfterBrowseInstance->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.BuildAfterBrowseInstance"));

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_GENERAL"));
	return true;
}

void GeneralSettingsPage::destroy()
{
}

bool GeneralSettingsPage::apply(PropertyGroup* settings)
{
	settings->setProperty< PropertyString >(L"Editor.Dictionary", m_editDictionary->getText());
	settings->setProperty< PropertyString >(L"Editor.StyleSheet", m_editStyleSheet->getText());
	settings->setProperty< PropertyString >(L"Editor.Font", m_dropFonts->getSelectedItem());
	settings->setProperty< PropertyInteger >(L"Editor.FontSize", parseString< int32_t >(m_editFontSize->getText()));
	settings->setProperty< PropertyBoolean >(L"Editor.AutoOpenRecentlyUsedWorkspace", m_checkAutoOpen->isChecked());
	settings->setProperty< PropertyBoolean >(L"Editor.AutoSave", m_checkAutoSave->isChecked());
	settings->setProperty< PropertyBoolean >(L"Editor.BuildWhenSourceModified", m_checkBuildWhenSourceModified->isChecked());
	settings->setProperty< PropertyBoolean >(L"Editor.BuildWhenAssetModified", m_checkBuildWhenAssetModified->isChecked());
	settings->setProperty< PropertyBoolean >(L"Editor.BuildAfterBrowseInstance", m_checkBuildAfterBrowseInstance->isChecked());
	return true;
}

	}
}
