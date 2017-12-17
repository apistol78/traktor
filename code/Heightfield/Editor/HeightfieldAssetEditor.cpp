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
#include "UI/Application.h"
#include "UI/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "UI/Image.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{
		
Ref< drawing::Image > generatePreviewImage(const Heightfield* hf)
{
	Ref< drawing::Image > img = new drawing::Image(drawing::PixelFormat::getR8G8B8(), hf->getSize(), hf->getSize());

	const Color4f c_colorValley(1.0f, 0.0f, 0.0f, 1.0f);
	const Color4f c_colorPeak(0.0f, 0.0f, 1.0f, 1.0f);

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
	containerInner->create(parent, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 4, 4));

	Ref< ui::Static > staticExtent = new ui::Static();
	staticExtent->create(containerInner, i18n::Text(L"HEIGHTFIELD_ASSET_EXTENT"));

	Ref< ui::Container > containerExtent = new ui::Container();
	containerExtent->create(containerInner, ui::WsNone, new ui::TableLayout(L"*,*,*", L"*", 0, 4));

	m_editExtent[0] = new ui::Edit();
	m_editExtent[0]->create(containerExtent, toString(m_asset->getWorldExtent().x()), ui::WsClientBorder, new ui::NumericEditValidator(true, 1.0f));

	m_editExtent[1] = new ui::Edit();
	m_editExtent[1]->create(containerExtent, toString(m_asset->getWorldExtent().y()), ui::WsClientBorder, new ui::NumericEditValidator(true, 1.0f));

	m_editExtent[2] = new ui::Edit();
	m_editExtent[2]->create(containerExtent, toString(m_asset->getWorldExtent().z()), ui::WsClientBorder, new ui::NumericEditValidator(true, 1.0f));

	Ref< ui::Static > staticVista = new ui::Static();
	staticVista->create(containerInner, i18n::Text(L"HEIGHTFIELD_ASSET_VISTA"));

	m_editVista = new ui::Edit();
	m_editVista->create(containerInner, toString(m_asset->getVistaDistance()), ui::WsClientBorder, new ui::NumericEditValidator(true, 1.0f));


	Ref< ui::Static > staticSize = new ui::Static();
	staticSize->create(containerInner, i18n::Text(L"HEIGHTFIELD_ASSET_SIZE"));

	m_editSize = new ui::Edit();
	m_editSize->create(containerInner, toString(m_heightfield->getSize()), ui::WsClientBorder, new ui::NumericEditValidator(false, 1.0f));


	Ref< ui::Static > staticPreview = new ui::Static();
	staticPreview->create(containerInner, i18n::Text(L"HEIGHTFIELD_ASSET_PREVIEW"));


	Ref< ui::Image > imagePreview = new ui::Image();
	imagePreview->create(containerInner, new ui::Bitmap(
		generatePreviewImage(m_heightfield)
	));


	return true;
}

void HeightfieldAssetEditor::destroy()
{
	m_instance = 0;
	m_asset = 0;
}

void HeightfieldAssetEditor::apply()
{
}

bool HeightfieldAssetEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void HeightfieldAssetEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size HeightfieldAssetEditor::getPreferredSize() const
{
	return ui::Size(
		ui::scaleBySystemDPI(850),
		ui::scaleBySystemDPI(550)
	);
}

	}
}
