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

	// World extent.
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

	// Tile count.
	Ref< ui::Static > staticTiles = new ui::Static();
	staticTiles->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_TILES"));

	m_editTiles = new ui::Edit();
	m_editTiles->create(containerFields, L"", ui::WsNone, new ui::NumericEditValidator(false, 0, c_maxTileCount));
	m_editTiles->addEventHandler< ui::FocusEvent >(this, &HeightfieldAssetEditor::eventFieldFocus);
	m_editTiles->addEventHandler< ui::KeyDownEvent >(this, &HeightfieldAssetEditor::eventFieldKeyDown);

	Ref< ui::Static > staticTileUnit = new ui::Static();
	staticTileUnit->create(containerFields, i18n::Text(L"HEIGHTFIELD_ASSET_TILE_UNIT"));

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

	ui::InputDialog::Field fields[] =
	{
		ui::InputDialog::Field(L"Component", L"", nullptr, new ui::InputDialog::KeyValueArrayEnumerator(layer == LyHeights ? c_rawBitsHeights : c_rawBits)),
		ui::InputDialog::Field(L"Byte order", L"", nullptr, new ui::InputDialog::StringArrayEnumerator(c_rawByteOrders)),
		ui::InputDialog::Field(L"Sign", L"", nullptr, new ui::InputDialog::StringArrayEnumerator(c_rawSigns)),
		ui::InputDialog::Field(L"Horizontal alignment", L"", nullptr, new ui::InputDialog::StringArrayEnumerator(c_rawHorizontalAlignments)),
		ui::InputDialog::Field(L"Vertical alignment", L"", nullptr, new ui::InputDialog::StringArrayEnumerator(c_rawVerticalAlignments)),
		ui::InputDialog::Field(L"Size", L"", nullptr, new ui::InputDialog::StringArrayEnumerator(c_rawSizes)),
		ui::InputDialog::Field(L"Y range (m)", L"0", new ui::NumericEditValidator(true, 0.0f))
	};

	// Y range only applies to heights, thus last field is only shown for heights.
	ui::InputDialog inputDialog;
	inputDialog.create(
		m_container,
		i18n::Text(L"HEIGHTFIELD_ASSET_IMPORT_RAW_TITLE"),
		i18n::Text(L"HEIGHTFIELD_ASSET_IMPORT_RAW_MESSAGE"),
		fields,
		sizeof_array(fields) - ((layer == LyHeights) ? 0 : 1)
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

	// Raw zero is at zero metres and full raw range spans Y range, mapped into world extent.
	const float yRange = parseString< float >(fields[6].value, 0.0f);
	const float worldHeight = m_asset->getWorldExtent().y();
	const float heightScale = (yRange > 0.0f) ? yRange / worldHeight : 1.0f;
	const float heightOffset = (yRange > 0.0f) ? 0.5f - zero * heightScale : 0.0f;
	if (yRange > 0.0f && (zero * yRange > worldHeight * 0.5f || (1.0f - zero) * yRange > worldHeight * 0.5f))
		log::warning << L"Raw Y range exceeds world extent; heights outside of world extent are clamped." << Endl;

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

bool HeightfieldAssetEditor::resizeTiles(int32_t tileCount)
{
	const int32_t oldTileCount = m_asset->getTileCount();
	const int32_t oldCount = std::max(oldTileCount, 1);
	const int32_t count = std::max(tileCount, 1);
	const Vector4 oldWorldExtent = m_asset->getWorldExtent();
	const Vector4 worldExtent(oldWorldExtent.x() * count / oldCount, oldWorldExtent.y(), oldWorldExtent.z() * count / oldCount, 0.0f);

	const TileStorage source(m_instance, oldTileCount);
	const TileStorage target(m_instance, tileCount);

	// Kept tiles are rewritten when data of removed tiles must be dropped or when channels are named differently.
	const bool rewrite = (count < oldCount) || ((oldTileCount > 0) != (tileCount > 0));
	if (rewrite && !m_instance->removeAllData())
		return false;

	Ref< Heightfield > flat = new Heightfield(m_tileSize, worldExtent, Vector4::zero());
	for (int32_t z = 0; z < m_tileSize; ++z)
	{
		for (int32_t x = 0; x < m_tileSize; ++x)
		{
			flat->setGridHeight(x, z, 0.5f);
			flat->setGridCut(x, z, true);
		}
	}

	for (int32_t tz = 0; tz < count; ++tz)
	{
		// Only new tiles are written in rows of kept tiles, unless kept tiles are rewritten.
		const int32_t tx0 = (rewrite || tz >= oldCount) ? 0 : oldCount;
		if (tx0 >= count)
			continue;

		RefArray< const Heightfield > tiles(count - tx0);
		AlignedVector< Job::task_t > tasks;
		for (int32_t tx = tx0; tx < count; ++tx)
			tasks.push_back([&, tx]() {
				tiles[tx - tx0] = (tx < oldCount && tz < oldCount) ? source.readStoredTile(oldWorldExtent, tx, tz) : flat;
			});
		JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

		for (const Heightfield* tile : tiles)
			if (!tile)
				return false;

		if (!target.writeStoredTiles(tx0, tz, count - 1, tz, tiles))
			return false;
	}

	m_asset->setTileCount(tileCount);
	m_asset->setWorldExtent(worldExtent);

	m_selectedTileX = std::min(m_selectedTileX, count - 1);
	m_selectedTileZ = std::min(m_selectedTileZ, count - 1);

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
	const int32_t newTileCount = clamp(parseString< int32_t >(m_editTiles->getText(), tileCount), 0, c_maxTileCount);

	if (newTileCount != tileCount && !resizeTiles(newTileCount))
		log::error << L"Unable to change number of heightfield tiles." << Endl;

	// Horizontal extent only scales and repositions existing tiles.
	if (x > 0.0f && z > 0.0f && (std::abs(x - worldExtent.x()) > c_extentEpsilon || std::abs(z - worldExtent.z()) > c_extentEpsilon))
	{
		const Vector4& currentWorldExtent = m_asset->getWorldExtent();
		m_asset->setWorldExtent(Vector4(x, currentWorldExtent.y(), z, 0.0f));
	}

	// Heights are re-encoded to keep world heights in new vertical extent.
	if (y > 0.0f && std::abs(y - worldExtent.y()) > c_extentEpsilon)
	{
		const float scale = worldExtent.y() / y;
		if (scaleHeights(scale, 0.5f - 0.5f * scale))
		{
			const Vector4& currentWorldExtent = m_asset->getWorldExtent();
			m_asset->setWorldExtent(Vector4(currentWorldExtent.x(), y, currentWorldExtent.z(), 0.0f));
		}
		else
			log::error << L"Unable to change vertical extent of heightfield." << Endl;
	}

	updateFields();
}

void HeightfieldAssetEditor::updateFields()
{
	const Vector4& worldExtent = m_asset->getWorldExtent();
	m_editExtent[0]->setText(toString(worldExtent.x()));
	m_editExtent[1]->setText(toString(worldExtent.y()));
	m_editExtent[2]->setText(toString(worldExtent.z()));
	m_editTiles->setText(toString(m_asset->getTileCount()));
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
