/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/ConvertHeightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldAssetEditor.h"
#include "I18N/Text.h"
#include "Model/ModelFormat.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/FileDialog.h"
#include "Ui/Image.h"
#include "Ui/InputDialog.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarSeparator.h"

namespace traktor::hf
{
	namespace
	{

Ref< drawing::Image > generatePreviewImage(const Heightfield* hf)
{
	Ref< drawing::Image > img = new drawing::Image(drawing::PixelFormat::getR8G8B8(), hf->getSize(), hf->getSize());
	for (int32_t y = 0; y < hf->getSize(); ++y)
	{
		for (int32_t x = 0; x < hf->getSize(); ++x)
		{
			float h = 1.0f - hf->getGridHeightNearest(x, y);

			uint8_t rgb[3] = { 0, 0, 0 };
			if (h <= 0.5f)
			{
				h *= 2.0f;
				rgb[0] = (uint8_t)(255 * (1.0f - h) + 0.5f);
				rgb[1] = (uint8_t)(255 * h + 0.5f);
			}
			else
			{
				h = h * 2.0f - 1.0f;
				rgb[1] = (uint8_t)(255 * (1.0f - h) + 0.5f);
				rgb[2] = (uint8_t)(255 * h + 0.5f);
			}

			const Color4f c = Color4f::fromColor4ub(Color4ub(rgb[0], rgb[1], rgb[2], 255));
			img->setPixel(x, y, c);
		}
	}
	return img;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.HeightfieldAssetEditor", HeightfieldAssetEditor, editor::IObjectEditor)

HeightfieldAssetEditor::HeightfieldAssetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool HeightfieldAssetEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	m_asset = mandatory_non_null_type_cast< HeightfieldAsset* >(object);

	Ref< IStream > sourceData = m_instance->readData(L"Data");
	if (!sourceData)
	{
		log::error << L"Heightfield asset editor failed; unable to open heights." << Endl;
		return false;
	}

	m_heightfield = HeightfieldFormat().read(
		sourceData,
		m_asset->getWorldExtent()
	);
	if (!m_heightfield)
	{
		log::error << L"Heightfield asset editor failed; unable to read heights." << Endl;
		return false;
	}

	sourceData->close();
	sourceData = nullptr;

	m_container = new ui::Container();
	m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,*,100%", 4_ut, 4_ut));

	Ref< ui::Container > containerFields = new ui::Container();
	containerFields->create(m_container, ui::WsNone, new ui::TableLayout(L"*,*,*", L"*", 0_ut, 4_ut));

	Ref< ui::Static > staticExtent = new ui::Static();
	staticExtent->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_EXTENT"));

	Ref< ui::Container > containerExtent = new ui::Container();
	containerExtent->create(containerFields, ui::WsNone, new ui::TableLayout(L"*,*,*", L"*", 0_ut, 4_ut));

	m_editExtent[0] = new ui::Edit();
	m_editExtent[0]->create(containerExtent, toString(m_asset->getWorldExtent().x()), ui::WsNone, new ui::NumericEditValidator(true, 1.0f));

	m_editExtent[1] = new ui::Edit();
	m_editExtent[1]->create(containerExtent, toString(m_asset->getWorldExtent().y()), ui::WsNone, new ui::NumericEditValidator(true, 1.0f));

	m_editExtent[2] = new ui::Edit();
	m_editExtent[2]->create(containerExtent, toString(m_asset->getWorldExtent().z()), ui::WsNone, new ui::NumericEditValidator(true, 1.0f));

	Ref< ui::Static > staticExtentUnit = new ui::Static();
	staticExtentUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_EXTENT_UNIT"));

	Ref< ui::Static > staticSize = new ui::Static();
	staticSize->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_SIZE"));

	m_editSize = new ui::Edit();
	m_editSize->create(containerFields, toString(m_heightfield->getSize()), ui::Edit::WsReadOnly);

	Ref< ui::Static > staticSizeUnit = new ui::Static();
	staticSizeUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_SIZE_UNIT"));

	Ref< ui::ToolBar > toolBar = new ui::ToolBar();
	toolBar->create(m_container);
	toolBar->addItem(new ui::ToolBarButton(L"Import...", ui::Command(L"HeightfieldAssetEditor.Import")));
	toolBar->addItem(new ui::ToolBarButton(L"Export...", ui::Command(L"HeightfieldAssetEditor.Export")));
	toolBar->addItem(new ui::ToolBarButton(L"Export as mesh...", ui::Command(L"HeightfieldAssetEditor.ExportAsMesh")));
	toolBar->addItem(new ui::ToolBarSeparator());
	toolBar->addItem(new ui::ToolBarButton(L"Clear...", ui::Command(L"HeightfieldAssetEditor.Clear")));
	toolBar->addItem(new ui::ToolBarButton(L"Resize...", ui::Command(L"HeightfieldAssetEditor.Resize")));
	toolBar->addItem(new ui::ToolBarButton(L"Crop...", ui::Command(L"HeightfieldAssetEditor.Crop")));
	toolBar->addItem(new ui::ToolBarButton(L"Remap height...", ui::Command(L"HeightfieldAssetEditor.RemapHeight")));
	toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &HeightfieldAssetEditor::eventToolBar);

	m_imagePreview = new ui::Image();
	m_imagePreview->create(
		m_container,
		new ui::Bitmap(generatePreviewImage(m_heightfield)),
		ui::WsDoubleBuffer | ui::Image::WsScaleKeepAspect
	);

	return true;
}

void HeightfieldAssetEditor::destroy()
{
	m_instance = nullptr;
	m_asset = nullptr;
}

void HeightfieldAssetEditor::apply()
{
	Ref< IStream > sourceData = m_instance->writeData(L"Data");
	if (sourceData)
	{
		if (!HeightfieldFormat().write(
			sourceData,
			m_heightfield
		))
			log::error << L"Unable to write heightfield to instance data stream." << Endl;

		sourceData->close();
	}
	else
		log::error << L"Unable to write data to instance." << Endl;
}

bool HeightfieldAssetEditor::handleCommand(const ui::Command& command)
{
	if (command == L"HeightfieldAssetEditor.Export")
	{
		Path fileName;

		ui::FileDialog saveAsDialog;
		saveAsDialog.create(m_container, type_name(this), i18n::Text(L"EXPORT_HEIGHTFIELD_WIZARDTOOL_FILE_TITLE"), L"All files;*.*", L"", true);
		if (saveAsDialog.showModal(fileName) != ui::DialogResult::Ok)
		{
			saveAsDialog.destroy();
			return false;
		}

		saveAsDialog.destroy();

		Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
		if (!file)
			return false;

		file->write(
			m_heightfield->getHeights(),
			m_heightfield->getSize() * m_heightfield->getSize() * sizeof(height_t)
		);

		file->close();
		return true;
	}
	else if (command == L"HeightfieldAssetEditor.ExportAsMesh")
	{
		Path fileName;

		ui::FileDialog saveAsDialog;
		saveAsDialog.create(m_container, type_name(this), i18n::Text(L"EXPORT_HEIGHTFIELD_AS_MESH_WIZARDTOOL_FILE_TITLE"), L"All files;*.*", L"", true);
		if (saveAsDialog.showModal(fileName) != ui::DialogResult::Ok)
		{
			saveAsDialog.destroy();
			return false;
		}

		saveAsDialog.destroy();

		Ref< model::Model > model = ConvertHeightfield().convert(m_heightfield, 16);
		if (!model)
			return false;

		return model::ModelFormat::writeAny(fileName, model);		
	}
	else if (command == L"HeightfieldAssetEditor.Clear")
	{
		const int32_t size = m_heightfield->getSize();
		for (int32_t z = 0; z < size; ++z)
		{
			for (int32_t x = 0; x < size; ++x)
			{
				m_heightfield->setGridHeight(x, z, 0.5f);
				m_heightfield->setGridCut(x, z, true);
			}
		}
		updatePreviewImage();
		return true;
	}
	else if (command == L"HeightfieldAssetEditor.Resize")
	{
		ui::InputDialog::Field fields[] =
		{
			ui::InputDialog::Field(L"Size", toString(m_heightfield->getSize()), new ui::NumericEditValidator(false, 1))
		};

		ui::InputDialog inputDialog;
		inputDialog.create(
			m_container,
			i18n::Text(L"CROP_HEIGHTFIELD_WIZARDTOOL_TITLE"),
			i18n::Text(L"CROP_HEIGHTFIELD_WIZARDTOOL_MESSAGE"),
			fields,
			sizeof_array(fields)
		);

		if (inputDialog.showModal() == ui::DialogResult::Cancel)
		{
			inputDialog.destroy();
			return false;
		}

		const int32_t size = parseString< int32_t >(fields[0].value);

		inputDialog.destroy();

		if (size <= 0)
		{
			log::error << L"Invalid size; must at least be greater or equal to one." << Endl;
			return false;
		}

		Ref< Heightfield > resized = new Heightfield(
			size,
			m_heightfield->getWorldExtent()
		);

		for (int32_t iy = 0; iy < size; ++iy)
		{
			for (int32_t ix = 0; ix < size; ++ix)
			{
				float sx = (float)(ix * m_heightfield->getSize()) / size;
				float sy = (float)(iy * m_heightfield->getSize()) / size;

				sx = clamp(sx, 0.0f, (float)(m_heightfield->getSize() - 1));
				sy = clamp(sy, 0.0f, (float)(m_heightfield->getSize() - 1));

				const float h = m_heightfield->getGridHeightBilinear(sx, sy);
				const bool c = m_heightfield->getGridCut((int32_t)sx, (int32_t)sy);

				resized->setGridHeight(ix, iy, h);
				resized->setGridCut(ix, iy, c);
			}
		}

		m_heightfield = resized;

		m_editSize->setText(toString(m_heightfield->getSize()));
		updatePreviewImage();

		return true;
	}
	else if (command == L"HeightfieldAssetEditor.Crop")
	{
		ui::InputDialog::Field fields[] =
		{
			ui::InputDialog::Field(L"Left", L"0", new ui::NumericEditValidator(false)),
			ui::InputDialog::Field(L"Top", L"0", new ui::NumericEditValidator(false)),
			ui::InputDialog::Field(L"Size", toString(m_heightfield->getSize()), new ui::NumericEditValidator(false, 1))
		};

		ui::InputDialog inputDialog;
		inputDialog.create(
			m_container,
			i18n::Text(L"CROP_HEIGHTFIELD_WIZARDTOOL_TITLE"),
			i18n::Text(L"CROP_HEIGHTFIELD_WIZARDTOOL_MESSAGE"),
			fields,
			sizeof_array(fields)
		);

		if (inputDialog.showModal() == ui::DialogResult::Cancel)
		{
			inputDialog.destroy();
			return false;
		}

		const int32_t x = parseString< int32_t >(fields[0].value);
		const int32_t y = parseString< int32_t >(fields[1].value);
		const int32_t size = parseString< int32_t >(fields[2].value);

		inputDialog.destroy();

		if (size <= 0)
		{
			log::error << L"Invalid size; must at least be greater or equal to one." << Endl;
			return false;
		}

		Ref< Heightfield > cropped = new Heightfield(
			size,
			m_heightfield->getWorldExtent()
		);

		for (int32_t iy = 0; iy < size; ++iy)
		{
			for (int32_t ix = 0; ix < size; ++ix)
			{
				int32_t sx = ix + x;
				int32_t sy = iy + y;

				sx = clamp(sx, 0, m_heightfield->getSize() - 1);
				sy = clamp(sy, 0, m_heightfield->getSize() - 1);

				const float h = m_heightfield->getGridHeightNearest(sx, sy);
				const bool c = m_heightfield->getGridCut(sx, sy);

				cropped->setGridHeight(ix, iy, h);
				cropped->setGridCut(ix, iy, c);
			}
		}

		m_heightfield = cropped;
		m_editSize->setText(toString(m_heightfield->getSize()));

		updatePreviewImage();
		return true;
	}
	else if (command == L"HeightfieldAssetEditor.RemapHeight")
	{
		const Vector4 oldWorldExtent = m_heightfield->getWorldExtent();
		const Vector4 newWorldExtent = Vector4(oldWorldExtent.x(), 128.0f, oldWorldExtent.z());
		
		const float scale = oldWorldExtent.y() / newWorldExtent.y();
		const int32_t size = m_heightfield->getSize();

		Ref< Heightfield > cropped = new Heightfield(
			size,
			newWorldExtent
		);

		for (int32_t iy = 0; iy < size; ++iy)
		{
			for (int32_t ix = 0; ix < size; ++ix)
			{
				const float h = m_heightfield->unitToWorld(m_heightfield->getGridHeightNearest(ix, iy));
				const bool c = m_heightfield->getGridCut(ix, iy);

				cropped->setGridHeight(ix, iy, cropped->worldToUnit(h));
				cropped->setGridCut(ix, iy, c);
			}
		}

		m_heightfield = cropped;

		updatePreviewImage();
		return true;
	}
	else
		return false;
}

void HeightfieldAssetEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size HeightfieldAssetEditor::getPreferredSize() const
{
	return ui::Size(
		850,
		550
	);
}

void HeightfieldAssetEditor::updatePreviewImage()
{
	m_imagePreview->setImage(
		new ui::Bitmap(generatePreviewImage(m_heightfield)),
		false
	);
	m_container->update();
}

void HeightfieldAssetEditor::eventToolBar(ui::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

}
