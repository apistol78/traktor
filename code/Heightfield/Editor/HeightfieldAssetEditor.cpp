#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldAssetEditor.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/Image.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{
		
Ref< drawing::Image > generatePreviewImage(const Heightfield* hf)
{
	Ref< drawing::Image > img = new drawing::Image(drawing::PixelFormat::getR8G8B8(), hf->getSize(), hf->getSize());

	const Color4f c_colorValley(0.0f, 0.0f, 0.0f, 1.0f);
	const Color4f c_colorPeak(1.0f, 1.0f, 1.0f, 1.0f);

	for (int32_t y = 0; y < hf->getSize(); ++y)
	{
		for (int32_t x = 0; x < hf->getSize(); ++x)
		{
			float h = hf->getGridHeightNearest(x, y);
			Color4f c = c_colorValley * Scalar(1.0f - h) + c_colorPeak * Scalar(h);
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
		return 0;
	}

	sourceData->close();
	sourceData = 0;

	Ref< ui::Container > containerInner = new ui::Container();
	containerInner->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,*,100%", 4, 4));

	Ref< ui::Container > containerFields = new ui::Container();
	containerFields->create(containerInner, ui::WsNone, new ui::TableLayout(L"*,*,*", L"*", 0, 4));

	Ref< ui::Static > staticExtent = new ui::Static();
	staticExtent->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_EXTENT"));

	Ref< ui::Container > containerExtent = new ui::Container();
	containerExtent->create(containerFields, ui::WsNone, new ui::TableLayout(L"*,*,*", L"*", 0, 4));

	m_editExtent[0] = new ui::Edit();
	m_editExtent[0]->create(containerExtent, toString(m_asset->getWorldExtent().x()), ui::WsClientBorder, new ui::NumericEditValidator(true, 1.0f));

	m_editExtent[1] = new ui::Edit();
	m_editExtent[1]->create(containerExtent, toString(m_asset->getWorldExtent().y()), ui::WsClientBorder, new ui::NumericEditValidator(true, 1.0f));

	m_editExtent[2] = new ui::Edit();
	m_editExtent[2]->create(containerExtent, toString(m_asset->getWorldExtent().z()), ui::WsClientBorder, new ui::NumericEditValidator(true, 1.0f));

	Ref< ui::Static > staticExtentUnit = new ui::Static();
	staticExtentUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_EXTENT_UNIT"));

	Ref< ui::Static > staticVista = new ui::Static();
	staticVista->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_VISTA"));

	m_editVista = new ui::Edit();
	m_editVista->create(containerFields, toString(m_asset->getVistaDistance()), ui::WsClientBorder, new ui::NumericEditValidator(true, 1.0f));

	Ref< ui::Static > staticVistaUnit = new ui::Static();
	staticVistaUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_VISTA_UNIT"));

	Ref< ui::Static > staticSize = new ui::Static();
	staticSize->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_SIZE"));

	m_editSize = new ui::Edit();
	m_editSize->create(containerFields, toString(m_heightfield->getSize()), ui::WsClientBorder | ui::Edit::WsReadOnly, new ui::NumericEditValidator(false, 1.0f));

	Ref< ui::Static > staticSizeUnit = new ui::Static();
	staticSizeUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_SIZE_UNIT"));

	Ref< ui::custom::ToolBar > toolBar = new ui::custom::ToolBar();
	toolBar->create(containerInner);
	toolBar->addItem(new ui::custom::ToolBarButton(L"Import...", ui::Command(L"HeightfieldAssetEditor.Import")));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Export...", ui::Command(L"HeightfieldAssetEditor.Export")));
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(new ui::custom::ToolBarButton(L"Clear...", ui::Command(L"HeightfieldAssetEditor.Clear")));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Resize...", ui::Command(L"HeightfieldAssetEditor.Resize")));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Crop...", ui::Command(L"HeightfieldAssetEditor.Crop")));
	toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &HeightfieldAssetEditor::eventToolBar);

	m_imagePreview = new ui::Image();
	m_imagePreview->create(
		containerInner,
		new ui::Bitmap(generatePreviewImage(m_heightfield)),
		ui::WsDoubleBuffer | ui::Image::WsScaleKeepAspect
	);

	return true;
}

void HeightfieldAssetEditor::destroy()
{
	m_instance = 0;
	m_asset = 0;
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
	if (command == L"HeightfieldAssetEditor.Clear")
	{
		int32_t size = m_heightfield->getSize();
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
	else
		return false;
}

void HeightfieldAssetEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size HeightfieldAssetEditor::getPreferredSize() const
{
	return ui::Size(
		ui::dpi96(850),
		ui::dpi96(550)
	);
}

void HeightfieldAssetEditor::updatePreviewImage()
{
	m_imagePreview->setImage(
		new ui::Bitmap(generatePreviewImage(m_heightfield)),
		false
	);
}

void HeightfieldAssetEditor::eventToolBar(ui::custom::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

	}
}
