/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/App/ColorSettingsPage.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"
#include "Ui/ColorPicker/ColorDialog.h"
#include "Ui/PropertyList/ColorPropertyItem.h"
#include "Ui/PropertyList/PropertyCommandEvent.h"
#include "Ui/PropertyList/PropertyList.h"

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

bool ColorSettingsPage::create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 4)))
		return false;

	m_colorList = new ui::PropertyList();
	if (!m_colorList->create(container, ui::WsDoubleBuffer))
		return false;

	m_colorList->addEventHandler< ui::PropertyCommandEvent >(this, &ColorSettingsPage::eventPropertyCommand);
	m_colorList->setSeparator(ui::dpi96(200));

	Ref< PropertyGroup > colorGroup = settings->getProperty< PropertyGroup >(L"Editor.Colors");
	T_ASSERT(colorGroup);

	for (uint32_t i = 0; i < sizeof_array(c_propertyColors); ++i)
	{
		Color4f color = Color4f::fromColor4ub(colorGroup->getProperty< Color4ub >(c_propertyColors[i].prop));
		m_colorList->addPropertyItem(new ui::ColorPropertyItem(
			i18n::Text(c_propertyColors[i].name),
			color,
			false
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
	T_ASSERT(colorGroup);

	RefArray< ui::PropertyItem > propertyItems;
	m_colorList->getPropertyItems(propertyItems, 0);

	for (uint32_t i = 0; i < sizeof_array(c_propertyColors); ++i)
	{
		const ui::ColorPropertyItem* propertyItem = checked_type_cast< const ui::ColorPropertyItem* >(
			propertyItems[i]
		);

		Color4ub color = propertyItem->getValue().toColor4ub();

		colorGroup->setProperty< PropertyColor >(
			c_propertyColors[i].prop,
			color
		);
	}

	settings->setProperty(L"Editor.Colors", colorGroup);
	return true;
}

void ColorSettingsPage::eventPropertyCommand(ui::PropertyCommandEvent* event)
{
	Ref< ui::ColorPropertyItem > colorItem = dynamic_type_cast< ui::ColorPropertyItem* >(event->getItem());
	if (colorItem)
	{
		ui::ColorDialog colorDialog;
		colorDialog.create(
			m_colorList,
			i18n::Text(L"COLOR_DIALOG_TEXT"),
			ui::ColorDialog::WsDefaultFixed | ui::ColorDialog::WsAlpha,
			colorItem->getValue()
		);
		if (colorDialog.showModal() == ui::DialogResult::Ok)
		{
			colorItem->setValue(colorDialog.getColor());
			m_colorList->update();
		}
		colorDialog.destroy();
	}
}

	}
}
