#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/App/ColorSettingsPage.h"
#include "I18N/Text.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ColorPicker/ColorDialog.h"
#include "Ui/Custom/PropertyList/ColorPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyCommandEvent.h"
#include "Ui/Custom/PropertyList/PropertyList.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

const struct
{
	const wchar_t* name;
	const wchar_t* prop;
}
c_propertyColors[] =
{
	{ L"EDITOR_SETTINGS_COLOR_BACKGROUND", L"Background" },
	{ L"EDITOR_SETTINGS_COLOR_GRID", L"Grid" },
	{ L"EDITOR_SETTINGS_COLOR_BONE_WIRE", L"BoneWire" },
	{ L"EDITOR_SETTINGS_COLOR_BONE_WIRE_SEL", L"BoneWireSelected" },
	{ L"EDITOR_SETTINGS_COLOR_BBOX_WIRE", L"BoundingBoxWire" },
	{ L"EDITOR_SETTINGS_COLOR_BBOX_WIRE_SEL", L"BoundingBoxWireSelected" },
	{ L"EDITOR_SETTINGS_COLOR_BBOX_FACE_SEL", L"BoundingBoxFaceSelected" },
	{ L"EDITOR_SETTINGS_COLOR_SNAP_POINTS", L"SnapPoint" },
	{ L"EDITOR_SETTINGS_COLOR_REFERENCE_EDGES", L"ReferenceEdge" },
	{ L"EDITOR_SETTINGS_COLOR_CAMERA_WIRE", L"CameraWire" }
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.ColorSettingsPage", 0, ColorSettingsPage, ISettingsPage)

bool ColorSettingsPage::create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 4)))
		return false;

	m_colorList = new ui::custom::PropertyList();
	if (!m_colorList->create(container, ui::WsClientBorder | ui::WsDoubleBuffer))
		return false;

	m_colorList->addEventHandler< ui::custom::PropertyCommandEvent >(this, &ColorSettingsPage::eventPropertyCommand);
	m_colorList->setSeparator(200);

	Ref< PropertyGroup > colorGroup = settings->getProperty< PropertyGroup >(L"Editor.Colors");
	T_ASSERT (colorGroup);

	for (uint32_t i = 0; i < sizeof_array(c_propertyColors); ++i)
	{
		Color4ub color = colorGroup->getProperty< PropertyColor >(c_propertyColors[i].prop);
		m_colorList->addPropertyItem(new ui::custom::ColorPropertyItem(
			i18n::Text(c_propertyColors[i].name),
			color
		));
	}

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_COLOR"));
	return true;
}

void ColorSettingsPage::destroy()
{
}

bool ColorSettingsPage::apply(PropertyGroup* settings)
{
	Ref< PropertyGroup > colorGroup = settings->getProperty< PropertyGroup >(L"Editor.Colors");
	T_ASSERT (colorGroup);

	RefArray< ui::custom::PropertyItem > propertyItems;
	m_colorList->getPropertyItems(propertyItems, 0);

	for (uint32_t i = 0; i < sizeof_array(c_propertyColors); ++i)
	{
		const ui::custom::ColorPropertyItem* propertyItem = checked_type_cast< const ui::custom::ColorPropertyItem* >(
			propertyItems[i]
		);

		const Color4ub& color = propertyItem->getValue();

		colorGroup->setProperty< PropertyColor >(
			c_propertyColors[i].prop,
			color
		);
	}

	settings->setProperty(L"Editor.Colors", colorGroup);
	return true;
}

void ColorSettingsPage::eventPropertyCommand(ui::custom::PropertyCommandEvent* event)
{
	Ref< ui::custom::ColorPropertyItem > colorItem = dynamic_type_cast< ui::custom::ColorPropertyItem* >(event->getItem());
	if (colorItem)
	{
		ui::custom::ColorDialog colorDialog;
		colorDialog.create(
			m_colorList,
			i18n::Text(L"COLOR_DIALOG_TEXT"),
			ui::custom::ColorDialog::WsDefaultFixed | ui::custom::ColorDialog::WsAlpha,
			colorItem->getValue()
		);
		if (colorDialog.showModal() == ui::DrOk)
		{
			colorItem->setValue(colorDialog.getColor());
			m_colorList->update();
		}
		colorDialog.destroy();
	}
}

	}
}
