/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldAssetEditor.h"
#include "Heightfield/Editor/TileStorage.h"
#include "I18N/Text.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/DropDown.h"
#include "Ui/Edit.h"
#include "Ui/FileDialog.h"
#include "Ui/InputDialog.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/StyleSheet.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarSeparator.h"

#include <atomic>
#include <cmath>
#include <cstring>

namespace traktor::hf
{
	namespace
	{

enum Layer
{
	LyHeights,
	LyCuts,
	LySplats,
	LyColors
};

const wchar_t* c_rawBitsHeights[] = { L"16-bit", L"16", L"8-bit", L"8", L"32-bit", L"32", nullptr };
const wchar_t* c_rawBits[] = { L"8-bit", L"8", L"16-bit", L"16", L"32-bit", L"32", nullptr };
const wchar_t* c_rawByteOrders[] = { L"Little endian", L"Big endian", nullptr };
const wchar_t* c_rawSigns[] = { L"Unsigned", L"Signed", nullptr };
const wchar_t* c_rawHorizontalAlignments[] = { L"Left", L"Right", nullptr };
const wchar_t* c_rawVerticalAlignments[] = { L"Top", L"Bottom", nullptr };
const wchar_t* c_rawSizes[] = { L"Keep raw size", L"Resize to target", nullptr };
const int32_t c_maxPreviewSize = 1024;
const int32_t c_maxTileCount = 64;
const int32_t c_maxTileSize = 8192;
const float c_extentEpsilon = 0.01f;
const Color4f c_clearSplat(1.0f, 0.0f, 0.0f, 0.0f);
const Color4f c_clearColor(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 0.0f);

//! Low heights are blue, high heights are red.
Color4f heightColor(float unitHeight)
{
	const float h = 1.0f - unitHeight;
	if (h <= 0.5f)
		return Color4f(1.0f - h * 2.0f, h * 2.0f, 0.0f, 1.0f);
	else
		return Color4f(0.0f, 2.0f - h * 2.0f, h * 2.0f - 1.0f, 1.0f);
}

struct RawFormat
{
	int32_t bytes;
	bool bigEndian;
	bool isSigned;
};

//! Normalized value of raw component; signed values are offset so minimum is zero.
float decodeComponent(const uint8_t* data, const RawFormat& format)
{
	uint32_t value = 0;
	for (int32_t i = 0; i < format.bytes; ++i)
		value |= (uint32_t)data[i] << (8 * (format.bigEndian ? format.bytes - 1 - i : i));
	if (format.isSigned)
		value ^= 1u << (8 * format.bytes - 1);
	return (float)(value / (double)(0xffffffffu >> (32 - 8 * format.bytes)));
}

//! Read span of raw row, starting at column c0, as normalized components.
bool readRawRow(IStream* stream, const RawFormat& format, int32_t rawSize, int32_t components, int32_t row, int32_t c0, int32_t span, AlignedVector< uint8_t >& bytes, AlignedVector< float >& outRow)
{
	const int32_t sampleBytes = components * format.bytes;
	bytes.resize((size_t)span * sampleBytes);
	stream->seek(IStream::SeekSet, ((int64_t)row * rawSize + c0) * sampleBytes);
	if (stream->read(bytes.ptr(), bytes.size()) != (int64_t)bytes.size())
		return false;

	outRow.resize((size_t)span * components);
	for (size_t i = 0; i < outRow.size(); ++i)
		outRow[i] = decodeComponent(bytes.c_ptr() + i * format.bytes, format);
	return true;
}

//! Raw coordinate of target sample, either resized to fit or aligned to start or end; false if outside of raw.
bool rawCoordinate(int32_t target, int32_t targetSize, int32_t rawSize, bool resize, bool alignEnd, float& outRaw)
{
	if (resize)
	{
		outRaw = (float)clamp((target + 0.5) * rawSize / targetSize - 0.5, 0.0, (double)(rawSize - 1));
		return true;
	}
	outRaw = (float)(target - (alignEnd ? targetSize - rawSize : 0));
	return outRaw >= 0.0f && outRaw < (float)rawSize;
}

//! Copy region, width * height samples with all layers, from source at sx, sz into target at tx, tz.
void copyRegion(const Heightfield* source, int32_t sx, int32_t sz, Heightfield* target, int32_t tx, int32_t tz, int32_t width, int32_t height)
{
	const size_t sourceSize = source->getSize();
	const size_t targetSize = target->getSize();

	for (int32_t z = 0; z < height; ++z)
	{
		std::memcpy(target->getHeights() + tx + (tz + z) * targetSize, source->getHeights() + sx + (sz + z) * sourceSize, width * sizeof(height_t));
		for (int32_t x = 0; x < width; ++x)
			target->setGridCut(tx + x, tz + z, source->getGridCut(sx + x, sz + z));
	}

	for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
	{
		const uint8_t* sourceAttributes = source->getAttributes(a);
		if (!sourceAttributes)
			continue;

		uint8_t* targetAttributes = target->createAttributes(a);
		for (int32_t z = 0; z < height; ++z)
			std::memcpy(targetAttributes + tx + (tz + z) * targetSize, sourceAttributes + sx + (sz + z) * sourceSize, width);
	}

	if (const uint8_t* sourceSplats = source->getSplats())
	{
		uint8_t* targetSplats = target->createSplats();
		for (int32_t z = 0; z < height; ++z)
			std::memcpy(targetSplats + (tx + (tz + z) * targetSize) * 4, sourceSplats + (sx + (sz + z) * sourceSize) * 4, width * 4);
	}

	if (const uint8_t* sourceColors = source->getColors())
	{
		uint8_t* targetColors = target->createColors();
		for (int32_t z = 0; z < height; ++z)
			std::memcpy(targetColors + (tx + (tz + z) * targetSize) * 4, sourceColors + (sx + (sz + z) * sourceSize) * 4, width * 4);
	}
}

//! Two old samples, as tile and local coordinates, which are filtered into a new sample along one axis.
struct ResampleAxis
{
	int32_t tile[2];
	int32_t local[2];
	float f;
};

//! Old samples along one axis for tile size new samples starting at global sample n0; samples are cell centered.
void calculateResampleAxis(int32_t n0, int32_t tileSize, int32_t size, int32_t oldTileSize, int32_t oldSize, AlignedVector< ResampleAxis >& outAxis)
{
	outAxis.resize(tileSize);
	for (int32_t i = 0; i < tileSize; ++i)
	{
		const double u = clamp((n0 + i + 0.5) * oldSize / size - 0.5, 0.0, (double)(oldSize - 1));
		const int32_t s0 = (int32_t)u;
		const int32_t s1 = std::min(s0 + 1, oldSize - 1);
		outAxis[i] = { { s0 / oldTileSize, s1 / oldTileSize }, { s0 % oldTileSize, s1 % oldTileSize }, (float)(u - s0) };
	}
}

//! Resample new tile from count * count old tiles; all layers are bilinear filtered except cuts which are nearest.
Ref< Heightfield > resampleTile(const RefArray< const Heightfield >& oldTiles, int32_t count, const AlignedVector< ResampleAxis >& columns, const AlignedVector< ResampleAxis >& rows, const Vector4& worldExtent)
{
	const int32_t tileSize = (int32_t)columns.size();
	Ref< Heightfield > tile = new Heightfield(tileSize, worldExtent, Vector4::zero());

	// Optional layers are created when any contributing old tile has them.
	bool haveSplats = false;
	bool haveColors = false;
	bool haveAttributes[Heightfield::MaxAttributes] = { false };
	for (int32_t oz = rows.front().tile[0]; oz <= rows.back().tile[1]; ++oz)
	{
		for (int32_t ox = columns.front().tile[0]; ox <= columns.back().tile[1]; ++ox)
		{
			const Heightfield* old = oldTiles[ox + oz * count];
			haveSplats |= (old->getSplats() != nullptr);
			haveColors |= (old->getColors() != nullptr);
			for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
				haveAttributes[a] |= (old->getAttributes(a) != nullptr);
		}
	}

	for (int32_t z = 0; z < tileSize; ++z)
	{
		const ResampleAxis& r = rows[z];
		for (int32_t x = 0; x < tileSize; ++x)
		{
			const ResampleAxis& c = columns[x];
			const Heightfield* old[] =
			{
				oldTiles[c.tile[0] + r.tile[0] * count],
				oldTiles[c.tile[1] + r.tile[0] * count],
				oldTiles[c.tile[0] + r.tile[1] * count],
				oldTiles[c.tile[1] + r.tile[1] * count]
			};
			const int32_t lx[] = { c.local[0], c.local[1], c.local[0], c.local[1] };
			const int32_t lz[] = { r.local[0], r.local[0], r.local[1], r.local[1] };

			float h[4];
			for (int32_t i = 0; i < 4; ++i)
				h[i] = old[i]->getGridHeightNearestUnsafe(lx[i], lz[i]);
			tile->setGridHeight(x, z, lerp(lerp(h[0], h[1], c.f), lerp(h[2], h[3], c.f), r.f));

			const int32_t nearest = ((c.f < 0.5f) ? 0 : 1) + ((r.f < 0.5f) ? 0 : 2);
			tile->setGridCut(x, z, old[nearest]->getGridCut(lx[nearest], lz[nearest]));

			if (haveSplats)
			{
				const Color4f s0 = lerp(old[0]->getGridSplat(lx[0], lz[0]), old[1]->getGridSplat(lx[1], lz[1]), Scalar(c.f));
				const Color4f s1 = lerp(old[2]->getGridSplat(lx[2], lz[2]), old[3]->getGridSplat(lx[3], lz[3]), Scalar(c.f));
				tile->setGridSplat(x, z, lerp(s0, s1, Scalar(r.f)));
			}

			if (haveColors)
			{
				const Color4f c0 = lerp(old[0]->getGridColor(lx[0], lz[0]), old[1]->getGridColor(lx[1], lz[1]), Scalar(c.f));
				const Color4f c1 = lerp(old[2]->getGridColor(lx[2], lz[2]), old[3]->getGridColor(lx[3], lz[3]), Scalar(c.f));
				tile->setGridColor(x, z, lerp(c0, c1, Scalar(r.f)));
			}

			for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
			{
				if (!haveAttributes[a])
					continue;
				float d[4];
				for (int32_t i = 0; i < 4; ++i)
					d[i] = old[i]->getGridAttributeDensity(lx[i], lz[i], a);
				tile->setGridAttribute(x, z, a, (uint8_t)(lerp(lerp(d[0], d[1], c.f), lerp(d[2], d[3], c.f), r.f) + 0.5f));
			}
		}
	}

	return tile;
}

//! Smallest power of two step, dividing tile size, which keeps count tiles within preview size limit.
int32_t calculatePreviewStep(int32_t tileSize, int32_t count)
{
	int32_t step = 1;
	while ((tileSize % (step * 2)) == 0 && tileSize * count / step > c_maxPreviewSize)
		step *= 2;
	return step;
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

	const int32_t count = std::max(m_asset->getTileCount(), 1);
	m_tileSize = TileStorage(m_instance, m_asset->getTileCount()).readSize() / count;
	if (m_tileSize <= 0 || !readPreview() || !readTilePreview())
	{
		log::error << L"Heightfield asset editor failed; unable to read heights." << Endl;
		return false;
	}

	m_container = new ui::Container();
	m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,*,100%", 4_ut, 4_ut));

	Ref< ui::Container > containerFields = new ui::Container();
	containerFields->create(m_container, ui::WsNone, new ui::TableLayout(L"*,*,*", L"*", 0_ut, 4_ut));

	// World size.
	Ref< ui::Static > staticExtent = new ui::Static();
	staticExtent->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_EXTENT"));

	Ref< ui::Container > containerExtent = new ui::Container();
	containerExtent->create(containerFields, ui::WsNone, new ui::TableLayout(L"*,*,*", L"*", 0_ut, 4_ut));

	for (int32_t i = 0; i < 3; ++i)
	{
		m_editExtent[i] = new ui::Edit();
		m_editExtent[i]->create(containerExtent, L"", ui::WsNone, new ui::NumericEditValidator(true, 1.0f));
		m_editExtent[i]->addEventHandler< ui::FocusEvent >(this, &HeightfieldAssetEditor::eventFieldFocus);
		m_editExtent[i]->addEventHandler< ui::KeyDownEvent >(this, &HeightfieldAssetEditor::eventFieldKeyDown);
	}

	Ref< ui::Static > staticExtentUnit = new ui::Static();
	staticExtentUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_EXTENT_UNIT"));

	// Heights of entire heightfield.
	Ref< ui::Static > staticWorldHeights = new ui::Static();
	staticWorldHeights->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_WORLD_HEIGHTS"));

	m_staticWorldHeights = new ui::Static();
	m_staticWorldHeights->create(containerFields, L"");

	Ref< ui::Static > staticWorldHeightsUnit = new ui::Static();
	staticWorldHeightsUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_HEIGHTS_UNIT"));

	// Heights per metre along X; heightfield is resampled when changed.
	Ref< ui::Static > staticResolution = new ui::Static();
	staticResolution->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_RESOLUTION"));

	m_editResolution = new ui::Edit();
	m_editResolution->create(containerFields, L"", ui::WsNone, new ui::NumericEditValidator(true, 0.0f));
	m_editResolution->addEventHandler< ui::FocusEvent >(this, &HeightfieldAssetEditor::eventFieldFocus);
	m_editResolution->addEventHandler< ui::KeyDownEvent >(this, &HeightfieldAssetEditor::eventFieldKeyDown);

	Ref< ui::Static > staticResolutionUnit = new ui::Static();
	staticResolutionUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_RESOLUTION_UNIT"));

	// Tile count; listed by update fields.
	Ref< ui::Static > staticTiles = new ui::Static();
	staticTiles->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_TILES"));

	m_dropTiles = new ui::DropDown();
	m_dropTiles->create(containerFields);
	m_dropTiles->addEventHandler< ui::SelectionChangeEvent >(this, &HeightfieldAssetEditor::eventTilesChange);

	Ref< ui::Static > staticTileUnit = new ui::Static();
	staticTileUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_TILE_UNIT"));

	// Size of a tile, in metres and heights.
	Ref< ui::Static > staticTileSize = new ui::Static();
	staticTileSize->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_TILE_SIZE"));

	m_staticTileSize = new ui::Static();
	m_staticTileSize->create(containerFields, L"");

	Ref< ui::Static > staticTileSizeUnit = new ui::Static();
	staticTileSizeUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_EXTENT_UNIT"));

	Ref< ui::Static > staticTileHeights = new ui::Static();
	staticTileHeights->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_TILE_HEIGHTS"));

	m_staticTileHeights = new ui::Static();
	m_staticTileHeights->create(containerFields, L"");

	Ref< ui::Static > staticTileHeightsUnit = new ui::Static();
	staticTileHeightsUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_HEIGHTS_UNIT"));

	// Heights of overview, low resolution tile covering entire heightfield.
	Ref< ui::Static > staticOverviewHeights = new ui::Static();
	staticOverviewHeights->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_OVERVIEW_HEIGHTS"));

	m_staticOverviewHeights = new ui::Static();
	m_staticOverviewHeights->create(containerFields, L"");

	Ref< ui::Static > staticOverviewHeightsUnit = new ui::Static();
	staticOverviewHeightsUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_HEIGHTS_UNIT"));

	updateFields();

	m_toolLayer = new ui::ToolBarDropDown(ui::Command(L"HeightfieldAssetEditor.Layer"), 100_ut, L"Layer");
	m_toolLayer->add(L"Heights");
	m_toolLayer->add(L"Cuts");
	m_toolLayer->add(L"Splats");
	m_toolLayer->add(L"Colors");
	m_toolLayer->select(LyHeights);

	Ref< ui::ToolBar > toolBar = new ui::ToolBar();
	toolBar->create(m_container);
	toolBar->addItem(m_toolLayer);
	toolBar->addItem(new ui::ToolBarSeparator());
	toolBar->addItem(new ui::ToolBarButton(L"Import raw...", ui::Command(L"HeightfieldAssetEditor.ImportRaw")));
	toolBar->addItem(new ui::ToolBarButton(L"Import raw into selected tile...", ui::Command(L"HeightfieldAssetEditor.ImportRawTile")));
	toolBar->addItem(new ui::ToolBarSeparator());
	toolBar->addItem(new ui::ToolBarButton(L"Clear", ui::Command(L"HeightfieldAssetEditor.Clear")));
	toolBar->addItem(new ui::ToolBarButton(L"Clear selected tile", ui::Command(L"HeightfieldAssetEditor.ClearTile")));
	toolBar->addItem(new ui::ToolBarSeparator());
	toolBar->addItem(new ui::ToolBarButton(L"Remap heights...", ui::Command(L"HeightfieldAssetEditor.RemapHeights")));
	toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &HeightfieldAssetEditor::eventToolBar);

	Ref< ui::Container > containerPreviews = new ui::Container();
	containerPreviews->create(m_container, ui::WsNone, new ui::TableLayout(L"50%,50%", L"100%", 0_ut, 4_ut));

	m_previewControl = new ui::Widget();
	m_previewControl->create(containerPreviews, ui::WsDoubleBuffer);
	m_previewControl->addEventHandler< ui::PaintEvent >(this, &HeightfieldAssetEditor::eventPreviewPaint);
	m_previewControl->addEventHandler< ui::MouseButtonDownEvent >(this, &HeightfieldAssetEditor::eventPreviewButtonDown);

	m_tilePreviewControl = new ui::Widget();
	m_tilePreviewControl->create(containerPreviews, ui::WsDoubleBuffer);
	m_tilePreviewControl->addEventHandler< ui::PaintEvent >(this, &HeightfieldAssetEditor::eventTilePreviewPaint);

	updatePreviewImage();
	return true;
}

void HeightfieldAssetEditor::destroy()
{
	m_instance = nullptr;
	m_asset = nullptr;
}

void HeightfieldAssetEditor::apply()
{
	// Extent and tile count are applied as their fields are committed.
	m_instance->setObject(m_asset);
}

bool HeightfieldAssetEditor::handleCommand(const ui::Command& command)
{
	// Pending field edits are applied before any command.
	applyFields();

	const int32_t count = std::max(m_asset->getTileCount(), 1);

	if (command == L"HeightfieldAssetEditor.Layer")
		updatePreviewImage();
	else if (command == L"HeightfieldAssetEditor.ImportRaw")
		importRaw(0, 0, count - 1, count - 1);
	else if (command == L"HeightfieldAssetEditor.ImportRawTile")
		importRaw(m_selectedTileX, m_selectedTileZ, m_selectedTileX, m_selectedTileZ);
	else if (command == L"HeightfieldAssetEditor.Clear")
		clear(0, 0, count - 1, count - 1);
	else if (command == L"HeightfieldAssetEditor.ClearTile")
		clear(m_selectedTileX, m_selectedTileZ, m_selectedTileX, m_selectedTileZ);
	else if (command == L"HeightfieldAssetEditor.RemapHeights")
		remapHeights();
	else
		return false;

	return true;
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

bool HeightfieldAssetEditor::modifyTiles(int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1, const std::function< bool(Heightfield* tile, int32_t tileX, int32_t tileZ) >& modify)
{
	const TileStorage storage(m_instance, m_asset->getTileCount());
	const Vector4 worldExtent = m_asset->getWorldExtent();
	const int32_t previewTileSize = m_tileSize / m_previewStep;

	bool result = true;
	for (int32_t tz = tileZ0; tz <= tileZ1 && result; ++tz)
	{
		// Tiles of a row are modified in parallel.
		RefArray< const Heightfield > tiles(tileX1 - tileX0 + 1);
		AlignedVector< Job::task_t > tasks;
		for (int32_t tx = tileX0; tx <= tileX1; ++tx)
			tasks.push_back([&, tx]() {
				Ref< Heightfield > tile = storage.readStoredTile(worldExtent, tx, tz);
				if (!tile || !modify(tile, tx, tz))
					return;
				updatePreview(tile, m_previewStep, tx * previewTileSize, tz * previewTileSize, m_previewImages);
				if (tx == m_selectedTileX && tz == m_selectedTileZ)
					updatePreview(tile, m_tilePreviewStep, 0, 0, m_tilePreviewImages);
				tiles[tx - tileX0] = tile;
			});
		JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

		for (const Heightfield* tile : tiles)
			result &= (tile != nullptr);

		result = result && storage.writeStoredTiles(tileX0, tz, tileX1, tz, tiles);
	}

	updatePreviewImage();
	return result;
}

bool HeightfieldAssetEditor::importRaw(int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(m_container, type_name(this), i18n::Text(L"IMPORT_HEIGHTFIELD_WIZARDTOOL_FILE_TITLE"), L"All files;*.*", L""))
		return false;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return false;
	}
	fileDialog.destroy();

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
	{
		log::error << L"Unable to import raw; failed to open \"" << fileName.getPathName() << L"\"." << Endl;
		return false;
	}
	const int64_t fileSize = file->available();
	file->close();

	const int32_t layer = std::max(m_toolLayer->getSelected(), 0);
	const float worldHeight = m_asset->getWorldExtent().y();

	ui::InputDialog::Field fields[] =
	{
		ui::InputDialog::Field(L"Component", L"", nullptr, new ui::InputDialog::KeyValueArrayEnumerator(layer == LyHeights ? c_rawBitsHeights : c_rawBits)),
		ui::InputDialog::Field(L"Byte order", L"", nullptr, new ui::InputDialog::StringArrayEnumerator(c_rawByteOrders)),
		ui::InputDialog::Field(L"Sign", L"", nullptr, new ui::InputDialog::StringArrayEnumerator(c_rawSigns)),
		ui::InputDialog::Field(L"Horizontal alignment", L"", nullptr, new ui::InputDialog::StringArrayEnumerator(c_rawHorizontalAlignments)),
		ui::InputDialog::Field(L"Vertical alignment", L"", nullptr, new ui::InputDialog::StringArrayEnumerator(c_rawVerticalAlignments)),
		ui::InputDialog::Field(L"Size", L"", nullptr, new ui::InputDialog::StringArrayEnumerator(c_rawSizes)),
		ui::InputDialog::Field(L"Minimum height (m)", toString(-worldHeight * 0.5f), new ui::NumericEditValidator(true)),
		ui::InputDialog::Field(L"Maximum height (m)", toString(worldHeight * 0.5f), new ui::NumericEditValidator(true)),
		ui::InputDialog::Field(L"Ocean elevation (m)", L"0", new ui::NumericEditValidator(true))
	};

	// Height mapping only applies to heights, thus last three fields are only shown for heights.
	ui::InputDialog inputDialog;
	inputDialog.create(
		m_container,
		i18n::Text(L"HEIGHTFIELD_ASSET_IMPORT_RAW_TITLE"),
		i18n::Text(layer == LyHeights ? L"HEIGHTFIELD_ASSET_IMPORT_RAW_HEIGHTS_MESSAGE" : L"HEIGHTFIELD_ASSET_IMPORT_RAW_MESSAGE"),
		fields,
		sizeof_array(fields) - ((layer == LyHeights) ? 0 : 3)
	);

	if (inputDialog.showModal() == ui::DialogResult::Cancel)
	{
		inputDialog.destroy();
		return false;
	}

	inputDialog.destroy();

	const RawFormat format = { parseString< int32_t >(fields[0].value) / 8, fields[1].selectedIndex == 1, fields[2].selectedIndex == 1 };
	const bool alignRight = (fields[3].selectedIndex == 1);
	const bool alignBottom = (fields[4].selectedIndex == 1);
	const bool resize = (fields[5].selectedIndex == 1);

	const int32_t components = (layer == LySplats || layer == LyColors) ? 4 : 1;
	const int32_t sampleBytes = components * format.bytes;
	const int32_t size = (tileX1 - tileX0 + 1) * m_tileSize;
	const int32_t rawSize = (int32_t)std::sqrt((double)(fileSize / sampleBytes));
	if (rawSize <= 0 || (int64_t)rawSize * rawSize * sampleBytes != fileSize)
	{
		log::error << L"Unable to import raw; file isn't a square of " << sampleBytes << L" byte samples." << Endl;
		return false;
	}

	// Only tiles overlapped by raw are modified.
	int32_t range[] = { tileX0, tileZ0, tileX1, tileZ1 };
	if (!resize)
	{
		const int32_t ox = alignRight ? size - rawSize : 0;
		const int32_t oz = alignBottom ? size - rawSize : 0;
		range[0] = tileX0 + std::max(ox, 0) / m_tileSize;
		range[1] = tileZ0 + std::max(oz, 0) / m_tileSize;
		range[2] = tileX0 + (std::min(ox + rawSize, size) - 1) / m_tileSize;
		range[3] = tileZ0 + (std::min(oz + rawSize, size) - 1) / m_tileSize;
	}

	// Signed zero isn't zero when normalized.
	const uint8_t zeroBytes[] = { 0, 0, 0, 0 };
	const float zero = decodeComponent(zeroBytes, format);

	// Lowest and highest raw values are minimum and maximum height; ocean elevation, in same heights, is moved to world zero.
	const float minHeight = parseString< float >(fields[6].value, -worldHeight * 0.5f);
	const float maxHeight = parseString< float >(fields[7].value, worldHeight * 0.5f);
	const float oceanElevation = parseString< float >(fields[8].value, 0.0f);
	const float heightScale = (maxHeight - minHeight) / worldHeight;
	const float heightOffset = (minHeight - oceanElevation) / worldHeight + 0.5f;
	if (layer == LyHeights && (minHeight - oceanElevation < -worldHeight * 0.5f - c_extentEpsilon || maxHeight - oceanElevation > worldHeight * 0.5f + c_extentEpsilon))
		log::warning << L"Imported heights, " << minHeight - oceanElevation << L" to " << maxHeight - oceanElevation << L" m, exceed world size Y of " << worldHeight << L" m (+/- " << worldHeight * 0.5f << L" m); heights outside are clamped." << Endl;

	return modifyTiles(range[0], range[1], range[2], range[3], [&](Heightfield* tile, int32_t tileX, int32_t tileZ) {
		Ref< IStream > stream = FileSystem::getInstance().open(fileName, File::FmRead);
		if (!stream)
			return false;

		const int32_t x0 = (tileX - tileX0) * m_tileSize;
		const int32_t z0 = (tileZ - tileZ0) * m_tileSize;

		// Span of raw columns sampled by tile.
		float u0, u1;
		rawCoordinate(x0, size, rawSize, resize, alignRight, u0);
		rawCoordinate(x0 + m_tileSize - 1, size, rawSize, resize, alignRight, u1);
		const int32_t c0 = clamp((int32_t)std::floor(u0), 0, rawSize - 1);
		const int32_t c1 = clamp((int32_t)std::floor(u1) + 1, 0, rawSize - 1);

		// Rows are kept in slots by parity since bilinear sampling reads two adjacent rows.
		AlignedVector< uint8_t > bytes;
		AlignedVector< float > rows[2];
		int32_t rowIndices[] = { -1, -1 };

		for (int32_t z = 0; z < m_tileSize; ++z)
		{
			float v;
			if (!rawCoordinate(z0 + z, size, rawSize, resize, alignBottom, v))
				continue;

			const int32_t r0 = (int32_t)v;
			const float fv = v - r0;
			const int32_t r1 = (fv > 0.0f) ? r0 + 1 : r0;
			for (const int32_t r : { r0, r1 })
			{
				if (rowIndices[r & 1] == r)
					continue;

				if (!readRawRow(stream, format, rawSize, components, r, c0, c1 - c0 + 1, bytes, rows[r & 1]))
				{
					log::error << L"Unable to import raw; failed to read \"" << fileName.getPathName() << L"\"." << Endl;
					stream->close();
					return false;
				}
				rowIndices[r & 1] = r;
			}

			const float* row0 = rows[r0 & 1].c_ptr();
			const float* row1 = rows[r1 & 1].c_ptr();

			for (int32_t x = 0; x < m_tileSize; ++x)
			{
				float u;
				if (!rawCoordinate(x0 + x, size, rawSize, resize, alignRight, u))
					continue;

				const float fu = u - (int32_t)u;
				const int32_t k0 = ((int32_t)u - c0) * components;
				const int32_t k1 = (fu > 0.0f) ? k0 + components : k0;

				// Cuts are sampled nearest, everything else bilinear.
				if (layer == LyCuts)
				{
					const float* row = (fv < 0.5f) ? row0 : row1;
					tile->setGridCut(x, z, row[(fu < 0.5f) ? k0 : k1] != zero);
					continue;
				}

				float values[4];
				for (int32_t c = 0; c < components; ++c)
					values[c] = lerp(lerp(row0[k0 + c], row0[k1 + c], fu), lerp(row1[k0 + c], row1[k1 + c], fu), fv);

				if (layer == LyHeights)
					tile->setGridHeight(x, z, values[0] * heightScale + heightOffset);
				else if (layer == LySplats)
					tile->setGridSplat(x, z, Color4f(values[0], values[1], values[2], values[3]));
				else
					tile->setGridColor(x, z, Color4f(values[0], values[1], values[2], values[3]));
			}
		}

		stream->close();
		return true;
	});
}

bool HeightfieldAssetEditor::clear(int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1)
{
	const int32_t layer = std::max(m_toolLayer->getSelected(), 0);
	return modifyTiles(tileX0, tileZ0, tileX1, tileZ1, [&](Heightfield* tile, int32_t tileX, int32_t tileZ) {
		const bool haveLayer =
			(layer == LyHeights || layer == LyCuts) ||
			(layer == LySplats && tile->getSplats() != nullptr) ||
			(layer == LyColors && tile->getColors() != nullptr);
		if (!haveLayer)
			return true;

		for (int32_t z = 0; z < m_tileSize; ++z)
		{
			for (int32_t x = 0; x < m_tileSize; ++x)
				if (layer == LyHeights)
					tile->setGridHeight(x, z, 0.5f);
				else if (layer == LyCuts)
					tile->setGridCut(x, z, true);
				else if (layer == LySplats)
					tile->setGridSplat(x, z, c_clearSplat);
				else if (layer == LyColors)
					tile->setGridColor(x, z, c_clearColor);
		}
		return true;
	});
}

bool HeightfieldAssetEditor::remapHeights()
{
	const float height = m_asset->getWorldExtent().y();

	ui::InputDialog::Field fields[] =
	{
		ui::InputDialog::Field(L"From minimum", toString(-height * 0.5f), new ui::NumericEditValidator(true)),
		ui::InputDialog::Field(L"From maximum", toString(height * 0.5f), new ui::NumericEditValidator(true)),
		ui::InputDialog::Field(L"To minimum", toString(-height * 0.5f), new ui::NumericEditValidator(true)),
		ui::InputDialog::Field(L"To maximum", toString(height * 0.5f), new ui::NumericEditValidator(true))
	};

	ui::InputDialog inputDialog;
	inputDialog.create(
		m_container,
		i18n::Text(L"HEIGHTFIELD_ASSET_REMAP_HEIGHTS_TITLE"),
		i18n::Text(L"HEIGHTFIELD_ASSET_REMAP_HEIGHTS_MESSAGE"),
		fields,
		sizeof_array(fields)
	);

	if (inputDialog.showModal() == ui::DialogResult::Cancel)
	{
		inputDialog.destroy();
		return false;
	}

	inputDialog.destroy();

	const float fromMin = parseString< float >(fields[0].value);
	const float fromMax = parseString< float >(fields[1].value);
	const float toMin = parseString< float >(fields[2].value);
	const float toMax = parseString< float >(fields[3].value);
	if (fromMax <= fromMin)
	{
		log::error << L"Unable to remap heights; source range is empty." << Endl;
		return false;
	}

	// World heights are mapped linearly, expressed as a transform of unit heights.
	const float scale = (toMax - toMin) / (fromMax - fromMin);
	return scaleHeights(scale, (toMin - fromMin * scale) / height + 0.5f - 0.5f * scale);
}

bool HeightfieldAssetEditor::scaleHeights(float scale, float offset)
{
	const int32_t count = std::max(m_asset->getTileCount(), 1);
	return modifyTiles(0, 0, count - 1, count - 1, [&](Heightfield* tile, int32_t tileX, int32_t tileZ) {
		for (int32_t z = 0; z < m_tileSize; ++z)
			for (int32_t x = 0; x < m_tileSize; ++x)
				tile->setGridHeight(x, z, tile->getGridHeightNearestUnsafe(x, z) * scale + offset);
		return true;
	});
}

bool HeightfieldAssetEditor::redistributeTiles(int32_t tileCount)
{
	const int32_t oldTileCount = m_asset->getTileCount();
	const int32_t oldCount = std::max(oldTileCount, 1);
	const int32_t count = std::max(tileCount, 1);
	const int32_t size = m_tileSize * oldCount;
	if ((size % count) != 0)
	{
		log::error << L"Unable to change number of heightfield tiles; size " << size << L" isn't divisible into " << count << L" tiles." << Endl;
		return false;
	}

	const int32_t tileSize = size / count;
	const Vector4 worldExtent = m_asset->getWorldExtent();
	const TileStorage source(m_instance, oldTileCount);
	const TileStorage target(m_instance, tileCount);

	// Every channel is rewritten since tiles are named by their coordinates.
	if (!m_instance->removeAllData())
		return false;

	// New row tz overwrites old row tz; when tiles shrink rows are written bottom up so old rows are read first.
	const bool bottomUp = (tileSize < m_tileSize);

	// Old tiles are read when first overlapped by a new row and released once passed.
	RefArray< const Heightfield > oldTiles(oldCount * oldCount);
	for (int32_t i = 0; i < count; ++i)
	{
		const int32_t tz = bottomUp ? count - 1 - i : i;
		const int32_t z0 = tz * tileSize;
		const int32_t oz0 = z0 / m_tileSize;
		const int32_t oz1 = (z0 + tileSize - 1) / m_tileSize;

		for (int32_t j = 0; j < oldCount * oldCount; ++j)
			if (j / oldCount < oz0 || j / oldCount > oz1)
				oldTiles[j] = nullptr;

		for (int32_t oz = oz0; oz <= oz1; ++oz)
		{
			if (oldTiles[oz * oldCount] != nullptr)
				continue;

			AlignedVector< Job::task_t > tasks;
			for (int32_t ox = 0; ox < oldCount; ++ox)
				tasks.push_back([&, ox, oz]() {
					oldTiles[ox + oz * oldCount] = source.readStoredTile(worldExtent, ox, oz);
				});
			JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

			for (int32_t ox = 0; ox < oldCount; ++ox)
				if (oldTiles[ox + oz * oldCount] == nullptr)
					return false;
		}

		// New tiles of a row are composed from overlapping old tiles in parallel.
		RefArray< const Heightfield > tiles(count);
		AlignedVector< Job::task_t > tasks;
		for (int32_t tx = 0; tx < count; ++tx)
			tasks.push_back([&, tx]() {
				Ref< Heightfield > tile = new Heightfield(tileSize, worldExtent, Vector4::zero());
				const int32_t x0 = tx * tileSize;
				for (int32_t oz = oz0; oz <= oz1; ++oz)
				{
					for (int32_t ox = x0 / m_tileSize; ox <= (x0 + tileSize - 1) / m_tileSize; ++ox)
					{
						const int32_t ix0 = std::max(x0, ox * m_tileSize);
						const int32_t ix1 = std::min(x0 + tileSize, (ox + 1) * m_tileSize);
						const int32_t iz0 = std::max(z0, oz * m_tileSize);
						const int32_t iz1 = std::min(z0 + tileSize, (oz + 1) * m_tileSize);
						copyRegion(oldTiles[ox + oz * oldCount], ix0 - ox * m_tileSize, iz0 - oz * m_tileSize, tile, ix0 - x0, iz0 - z0, ix1 - ix0, iz1 - iz0);
					}
				}
				tiles[tx] = tile;
			});
		JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

		if (!target.writeStoredTiles(0, tz, count - 1, tz, tiles))
			return false;
	}

	m_tileSize = tileSize;
	m_asset->setTileCount(tileCount);

	m_selectedTileX = std::min(m_selectedTileX, count - 1);
	m_selectedTileZ = std::min(m_selectedTileZ, count - 1);

	if (!readPreview() || !readTilePreview())
		return false;

	updatePreviewImage();
	return true;
}

bool HeightfieldAssetEditor::resampleTiles(int32_t tileSize)
{
	const int32_t tileCount = m_asset->getTileCount();
	const int32_t count = std::max(tileCount, 1);
	const int32_t oldSize = m_tileSize * count;
	const int32_t size = tileSize * count;
	const Vector4 worldExtent = m_asset->getWorldExtent();
	const TileStorage storage(m_instance, tileCount);

	// New row tz overwrites old row tz; old rows are kept until no following row reference them.
	RefArray< const Heightfield > oldTiles(count * count);
	AlignedVector< ResampleAxis > rows;
	for (int32_t tz = 0; tz < count; ++tz)
	{
		calculateResampleAxis(tz * tileSize, tileSize, size, m_tileSize, oldSize, rows);
		const int32_t oz0 = rows.front().tile[0];
		const int32_t oz1 = rows.back().tile[1];

		for (int32_t j = 0; j < oz0 * count; ++j)
			oldTiles[j] = nullptr;

		for (int32_t oz = oz0; oz <= oz1; ++oz)
		{
			if (oldTiles[oz * count] != nullptr)
				continue;

			AlignedVector< Job::task_t > tasks;
			for (int32_t ox = 0; ox < count; ++ox)
				tasks.push_back([&, ox, oz]() {
					oldTiles[ox + oz * count] = storage.readStoredTile(worldExtent, ox, oz);
				});
			JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

			for (int32_t ox = 0; ox < count; ++ox)
				if (oldTiles[ox + oz * count] == nullptr)
					return false;
		}

		// New tiles of a row are resampled in parallel.
		RefArray< const Heightfield > tiles(count);
		AlignedVector< Job::task_t > tasks;
		for (int32_t tx = 0; tx < count; ++tx)
			tasks.push_back([&, tx]() {
				AlignedVector< ResampleAxis > columns;
				calculateResampleAxis(tx * tileSize, tileSize, size, m_tileSize, oldSize, columns);
				tiles[tx] = resampleTile(oldTiles, count, columns, rows, worldExtent);
			});
		JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

		if (!storage.writeStoredTiles(0, tz, count - 1, tz, tiles))
			return false;
	}

	m_tileSize = tileSize;

	if (!readPreview() || !readTilePreview())
		return false;

	updatePreviewImage();
	return true;
}

void HeightfieldAssetEditor::applyFields()
{
	const Vector4 worldExtent = m_asset->getWorldExtent();
	const int32_t tileCount = m_asset->getTileCount();

	// Fields being edited, thus empty, keep last valid value.
	const float x = parseString< float >(m_editExtent[0]->getText(), worldExtent.x());
	const float y = parseString< float >(m_editExtent[1]->getText(), worldExtent.y());
	const float z = parseString< float >(m_editExtent[2]->getText(), worldExtent.z());

	const float resolution = parseString< float >(m_editResolution->getText(), 0.0f);

	// First tile item is untiled.
	const int32_t tileItem = m_dropTiles->getSelected();
	const int32_t newTileCount = (tileItem > 0) ? parseString< int32_t >(m_dropTiles->getSelectedItem(), tileCount) : ((tileItem == 0) ? 0 : tileCount);

	if (newTileCount != tileCount && !redistributeTiles(newTileCount))
		log::error << L"Unable to change number of heightfield tiles." << Endl;

	// Resolution is relative to current world size, thus changing world size keep heights.
	const int32_t count = std::max(m_asset->getTileCount(), 1);
	const int32_t newTileSize = std::max((int32_t)std::round(worldExtent.x() * resolution / count), 1);
	if (resolution > 0.0f && newTileSize != m_tileSize)
	{
		if (newTileSize > c_maxTileSize)
			log::error << L"Unable to change heightfield resolution; tile would have " << newTileSize << L" heights, more than " << c_maxTileSize << L", use more tiles." << Endl;
		else if (!resampleTiles(newTileSize))
			log::error << L"Unable to change heightfield resolution." << Endl;
	}

	m_asset->setWorldExtent(Vector4(x, y, z, 0.0f));
	updateFields();
}

void HeightfieldAssetEditor::updateFields()
{
	const Vector4& worldExtent = m_asset->getWorldExtent();
	const int32_t tileCount = m_asset->getTileCount();
	const int32_t count = std::max(tileCount, 1);

	const int32_t size = m_tileSize * count;
	const int32_t overviewSize = TileStorage::calculateOverviewSize(size, worldExtent.x());

	m_editExtent[0]->setText(toString(worldExtent.x()));
	m_editExtent[1]->setText(toString(worldExtent.y()));
	m_editExtent[2]->setText(toString(worldExtent.z()));
	m_editResolution->setText(toString(size / worldExtent.x()));

	// Only tile counts which divide size are listed, first item is untiled.
	m_dropTiles->removeAll();
	m_dropTiles->add(i18n::Text(L"HEIGHTFIELD_ASSET_UNTILED"));
	for (int32_t i = 1; i <= c_maxTileCount; ++i)
		if ((size % i) == 0)
			m_dropTiles->add(toString(i));

	if (tileCount > 0)
		m_dropTiles->select(toString(tileCount));
	else
		m_dropTiles->select(0);

	m_staticWorldHeights->setText(toString(size) + L" x " + toString(size));
	m_staticTileSize->setText(toString(worldExtent.x() / count) + L" x " + toString(worldExtent.z() / count));
	m_staticTileHeights->setText(toString(m_tileSize) + L" x " + toString(m_tileSize));
	m_staticOverviewHeights->setText(toString(overviewSize) + L" x " + toString(overviewSize));

	// Tile information changes width of fields.
	m_staticTileSize->getParent()->update();
}

bool HeightfieldAssetEditor::readPreview()
{
	const TileStorage storage(m_instance, m_asset->getTileCount());
	const Vector4 worldExtent = m_asset->getWorldExtent();
	const int32_t count = std::max(m_asset->getTileCount(), 1);

	m_previewStep = calculatePreviewStep(m_tileSize, count);

	const int32_t previewTileSize = m_tileSize / m_previewStep;
	for (auto& previewImage : m_previewImages)
		previewImage = new drawing::Image(drawing::PixelFormat::getR8G8B8(), previewTileSize * count, previewTileSize * count);

	// Tiles of a row are read in parallel.
	std::atomic< bool > result = true;
	for (int32_t tz = 0; tz < count; ++tz)
	{
		AlignedVector< Job::task_t > tasks;
		for (int32_t tx = 0; tx < count; ++tx)
			tasks.push_back([&, tx]() {
				Ref< const Heightfield > tile = storage.readStoredTile(worldExtent, tx, tz);
				if (tile)
					updatePreview(tile, m_previewStep, tx * previewTileSize, tz * previewTileSize, m_previewImages);
				else
					result = false;
			});
		JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());
	}

	return result;
}

bool HeightfieldAssetEditor::readTilePreview()
{
	Ref< const Heightfield > tile = TileStorage(m_instance, m_asset->getTileCount()).readStoredTile(m_asset->getWorldExtent(), m_selectedTileX, m_selectedTileZ);
	if (!tile)
		return false;

	m_tilePreviewStep = calculatePreviewStep(m_tileSize, 1);

	const int32_t size = m_tileSize / m_tilePreviewStep;
	for (auto& tilePreviewImage : m_tilePreviewImages)
		tilePreviewImage = new drawing::Image(drawing::PixelFormat::getR8G8B8(), size, size);

	updatePreview(tile, m_tilePreviewStep, 0, 0, m_tilePreviewImages);
	return true;
}

void HeightfieldAssetEditor::updatePreview(const Heightfield* heightfield, int32_t step, int32_t x0, int32_t z0, Ref< drawing::Image >* images)
{
	const int32_t size = heightfield->getSize() / step;
	const int32_t n = step * step;

	for (int32_t z = 0; z < size; ++z)
	{
		for (int32_t x = 0; x < size; ++x)
		{
			float height = 0.0f;
			int32_t cuts = 0;
			Color4f splat(0.0f, 0.0f, 0.0f, 0.0f);
			Color4f color(0.0f, 0.0f, 0.0f, 0.0f);

			for (int32_t iz = z * step; iz < (z + 1) * step; ++iz)
			{
				for (int32_t ix = x * step; ix < (x + 1) * step; ++ix)
				{
					height += heightfield->getGridHeightNearestUnsafe(ix, iz);
					cuts += heightfield->getGridCut(ix, iz) ? 1 : 0;
					splat += heightfield->getGridSplat(ix, iz);
					color += heightfield->getGridColor(ix, iz);
				}
			}

			// Cut is majority of samples.
			const float solid = (cuts * 2 >= n) ? 1.0f : 0.0f;
			images[LyHeights]->setPixel(x0 + x, z0 + z, heightColor(height / n));
			images[LyCuts]->setPixel(x0 + x, z0 + z, Color4f(solid, solid, solid, 1.0f));
			images[LySplats]->setPixel(x0 + x, z0 + z, (splat / Scalar((float)n)).rgb1());
			images[LyColors]->setPixel(x0 + x, z0 + z, (color / Scalar((float)n)).rgb1());
		}
	}
}

void HeightfieldAssetEditor::updatePreviewImage()
{
	const int32_t layer = std::max(m_toolLayer->getSelected(), 0);
	m_previewBitmap = new ui::Bitmap(m_previewImages[layer]);
	m_tilePreviewBitmap = new ui::Bitmap(m_tilePreviewImages[layer]);
	m_previewControl->update();
	m_tilePreviewControl->update();
}

ui::Rect HeightfieldAssetEditor::getPreviewRect(const ui::Widget* control) const
{
	const ui::Rect rcInner = control->getInnerRect();
	const int32_t size = std::min(rcInner.getWidth(), rcInner.getHeight());
	return ui::Rect(
		ui::Point(rcInner.left + (rcInner.getWidth() - size) / 2, rcInner.top + (rcInner.getHeight() - size) / 2),
		ui::Size(size, size)
	);
}

void HeightfieldAssetEditor::paintPreview(ui::Canvas& canvas, ui::Widget* control, ui::Bitmap* bitmap) const
{
	const ui::StyleSheet* ss = control->getStyleSheet();

	canvas.setBackground(ss->getColor(control, L"background-color"));
	canvas.fillRect(control->getInnerRect());

	const ui::Rect rc = getPreviewRect(control);
	if (rc.getWidth() <= 0 || !bitmap)
		return;

	canvas.drawBitmap(
		rc.getTopLeft(),
		rc.getSize(),
		ui::Point(0, 0),
		bitmap->getSize(control),
		bitmap,
		ui::BlendMode::Opaque,
		ui::Filter::Linear
	);

	canvas.setForeground(ss->getColor(control, L"border-color"));
	canvas.drawRect(rc);
}

void HeightfieldAssetEditor::eventToolBar(ui::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void HeightfieldAssetEditor::eventFieldFocus(ui::FocusEvent* event)
{
	if (event->lostFocus())
		applyFields();
}

void HeightfieldAssetEditor::eventFieldKeyDown(ui::KeyDownEvent* event)
{
	if (event->getVirtualKey() == ui::VkReturn)
		applyFields();
}

void HeightfieldAssetEditor::eventTilesChange(ui::SelectionChangeEvent* event)
{
	applyFields();
}

void HeightfieldAssetEditor::eventPreviewPaint(ui::PaintEvent* event)
{
	ui::Canvas& canvas = event->getCanvas();
	const ui::StyleSheet* ss = m_previewControl->getStyleSheet();

	paintPreview(canvas, m_previewControl, m_previewBitmap);

	const ui::Rect rc = getPreviewRect(m_previewControl);
	if (rc.getWidth() <= 0)
		return;

	// Tile grid.
	const int32_t count = std::max(m_asset->getTileCount(), 1);
	canvas.setForeground(ss->getColor(m_previewControl, L"border-color"));
	for (int32_t i = 1; i < count; ++i)
	{
		const int32_t x = rc.left + rc.getWidth() * i / count;
		const int32_t y = rc.top + rc.getHeight() * i / count;
		canvas.drawLine(x, rc.top, x, rc.bottom);
		canvas.drawLine(rc.left, y, rc.right, y);
	}

	// Selected tile; outlined twice as to be visible on both bright and dark layers.
	const ui::Rect rcTile(
		rc.left + rc.getWidth() * m_selectedTileX / count,
		rc.top + rc.getHeight() * m_selectedTileZ / count,
		rc.left + rc.getWidth() * (m_selectedTileX + 1) / count,
		rc.top + rc.getHeight() * (m_selectedTileZ + 1) / count
	);
	canvas.setForeground(ss->getColor(m_previewControl, L"background-color"));
	canvas.drawRect(rcTile.inflate(1, 1));
	canvas.setForeground(ss->getColor(m_previewControl, L"color"));
	canvas.drawRect(rcTile);

	event->consume();
}

void HeightfieldAssetEditor::eventPreviewButtonDown(ui::MouseButtonDownEvent* event)
{
	const ui::Rect rc = getPreviewRect(m_previewControl);
	const ui::Point position = event->getPosition();
	if (rc.getWidth() <= 0 || !rc.inside(position, false))
		return;

	const int32_t count = std::max(m_asset->getTileCount(), 1);
	m_selectedTileX = clamp((position.x - rc.left) * count / rc.getWidth(), 0, count - 1);
	m_selectedTileZ = clamp((position.y - rc.top) * count / rc.getHeight(), 0, count - 1);

	readTilePreview();
	updatePreviewImage();

	event->consume();
}

void HeightfieldAssetEditor::eventTilePreviewPaint(ui::PaintEvent* event)
{
	paintPreview(event->getCanvas(), m_tilePreviewControl, m_tilePreviewBitmap);
	event->consume();
}

}
