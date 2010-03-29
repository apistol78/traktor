#include "Core/Misc/String.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Render/Editor/RenderSettingsPage.h"
#include "Ui/DropDown.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSettingsPage", 0, RenderSettingsPage, editor::ISettingsPage)

bool RenderSettingsPage::create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, 4)))
		return false;

	Ref< ui::Static > staticRenderer = new ui::Static();
	staticRenderer->create(container, i18n::Text(L"EDITOR_SETTINGS_RENDERER_TYPE"));

	m_dropRenderSystem = new ui::DropDown();
	m_dropRenderSystem->create(container, L"");

	Ref< ui::Static > staticMipBias = new ui::Static();
	staticMipBias->create(container, i18n::Text(L"EDITOR_SETTINGS_RENDERER_MIPBIAS"));

	m_editMipBias = new ui::Edit();
	m_editMipBias->create(container, L"", ui::WsClientBorder, new ui::NumericEditValidator(true, -100.0f, 100.0f));

	std::wstring renderSystemType = settings->getProperty< PropertyString >(L"Editor.RenderSystem");

	std::vector< const TypeInfo* > renderSystemTypes;
	type_of< render::IRenderSystem >().findAllOf(renderSystemTypes, false);

	for (std::vector< const TypeInfo* >::const_iterator i = renderSystemTypes.begin(); i != renderSystemTypes.end(); ++i)
	{
		std::wstring name = (*i)->getName();
		int32_t index = m_dropRenderSystem->add(name);
		if (name == renderSystemType)
			m_dropRenderSystem->select(index);
	}

	m_editMipBias->setText(toString(settings->getProperty< PropertyFloat >(L"Editor.MipBias")));

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_RENDERER"));
	return true;
}

void RenderSettingsPage::destroy()
{
}

bool RenderSettingsPage::apply(Settings* settings)
{
	settings->setProperty< PropertyString >(L"Editor.RenderSystem", m_dropRenderSystem->getSelectedItem());
	settings->setProperty< PropertyFloat >(L"Editor.MipBias", parseString< float >(m_editMipBias->getText()));
	return true;
}

	}
}
