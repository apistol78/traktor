/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/TrimSheetSetupEditor.h"

#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/PropertiesView.h"
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TrimSheetComposer.h"
#include "Render/Editor/Texture/TrimSheetControl.h"
#include "Render/Editor/Texture/TrimSheetSetupAsset.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/FileDialog.h"
#include "Ui/MenuItem.h"
#include "Ui/Splitter.h"
#include "Ui/StatusBar/StatusBar.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarMenu.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"

namespace traktor::render
{
namespace
{

const wchar_t* c_layerNames[] = { L"Albedo", L"Specular", L"Roughness", L"Normal", L"Height" };
const wchar_t* c_layerTextIds[] = { L"TRIMSHEET_EDITOR_LAYER_ALBEDO", L"TRIMSHEET_EDITOR_LAYER_SPECULAR", L"TRIMSHEET_EDITOR_LAYER_ROUGHNESS", L"TRIMSHEET_EDITOR_LAYER_NORMAL", L"TRIMSHEET_EDITOR_LAYER_HEIGHT" };

static_assert(sizeof_array(c_layerNames) == TrimSheetLayerCount, "Layer names out of sync with layers.");
static_assert(sizeof_array(c_layerTextIds) == TrimSheetLayerCount, "Layer texts out of sync with layers.");

/*! Identify what a structure tree item represents. */
class StructureItemData : public Object
{
	T_RTTI_CLASS;

public:
	explicit StructureItemData(int32_t slab, int32_t region)
	:	m_slab(slab)
	,	m_region(region)
	{
	}

	int32_t getSlab() const { return m_slab; }

	int32_t getRegion() const { return m_region; }

private:
	int32_t m_slab;
	int32_t m_region;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TrimSheetSetupEditor.StructureItemData", StructureItemData, Object)

std::wstring getRegionName(const TrimSheetRegion* region, int32_t slab, int32_t index)
{
	if (!region->getName().empty())
		return region->getName();
	return i18n::Format(L"TRIMSHEET_EDITOR_REGION", slab, index);
}

/*! Get layer whose image shows region's placement; current layer if it has an image, else first layer with an image. */
bool getPlacementLayer(const TrimSheetRegion* region, TrimSheetLayer currentLayer, TrimSheetLayer& outLayer)
{
	if (!region->getFileName(currentLayer).empty())
	{
		outLayer = currentLayer;
		return true;
	}
	for (int32_t i = 0; i < TrimSheetLayerCount; ++i)
	{
		if (!region->getFileName((TrimSheetLayer)i).empty())
		{
			outLayer = (TrimSheetLayer)i;
			return true;
		}
	}
	return false;
}

/*! Burn outline of each region into sheet image. */
void drawGuides(drawing::Image* sheet, const TrimSheetSetupAsset* setup)
{
	AlignedVector< TrimSheetRect > slabRects;
	AlignedVector< TrimSheetSetupAsset::RegionLayout > regionLayouts;
	setup->calculateLayout(slabRects, regionLayouts);

	const Color4f guideColor(1.0f, 0.0f, 1.0f, 1.0f);
	for (const auto& regionLayout : regionLayouts)
	{
		const TrimSheetRect& rc = regionLayout.rect;
		if (rc.empty())
			continue;

		for (int32_t x = rc.x; x < rc.x + rc.width; ++x)
		{
			sheet->setPixel(x, rc.y, guideColor);
			sheet->setPixel(x, rc.y + rc.height - 1, guideColor);
		}
		for (int32_t y = rc.y; y < rc.y + rc.height; ++y)
		{
			sheet->setPixel(rc.x, y, guideColor);
			sheet->setPixel(rc.x + rc.width - 1, y, guideColor);
		}
	}
}

bool saveImage(const drawing::Image* image, const Path& fileName)
{
	if (!image->save(fileName))
	{
		log::error << L"Unable to save trim sheet image \"" << fileName.getPathName() << L"\"." << Endl;
		return false;
	}
	log::info << L"Trim sheet image \"" << fileName.getPathName() << L"\" exported." << Endl;
	return true;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TrimSheetSetupEditor", TrimSheetSetupEditor, editor::IEditorPage)

TrimSheetSetupEditor::TrimSheetSetupEditor(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool TrimSheetSetupEditor::create(ui::Container* parent)
{
	m_asset = m_document->getObject< TrimSheetSetupAsset >(0);
	if (!m_asset)
		return false;

	m_assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	m_composer = new TrimSheetComposer(Path(m_assetPath));

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 0_ut, 0_ut));

	m_toolBar = new ui::ToolBar();
	m_toolBar->create(container);
	m_toolBar->addImage(new ui::StyleBitmap(L"Scene.RemoveEntity"));
	m_toolBar->addImage(new ui::StyleBitmap(L"Scene.MoveUpEntity"));
	m_toolBar->addImage(new ui::StyleBitmap(L"Scene.MoveDownEntity"));
	m_toolBar->addImage(new ui::StyleBitmap(L"Scene.Rotate"));
	m_toolBar->addImage(new ui::StyleBitmap(L"Scene.ToggleGuide"));

	m_toolLayer = new ui::ToolBarDropDown(ui::Command(L"Render.TrimSheet.Editor.Layer"), 100_ut, i18n::Text(L"TRIMSHEET_EDITOR_LAYER"));
	for (int32_t i = 0; i < TrimSheetLayerCount; ++i)
		m_toolLayer->add(i18n::Text(c_layerTextIds[i]));
	m_toolLayer->select((int32_t)m_layer);

	m_toolToggleGuides = new ui::ToolBarButton(i18n::Text(L"TRIMSHEET_EDITOR_TOGGLE_GUIDES"), 4, ui::Command(L"Render.TrimSheet.Editor.ToggleGuides"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleNames = new ui::ToolBarButton(i18n::Text(L"TRIMSHEET_EDITOR_TOGGLE_NAMES"), ui::Command(L"Render.TrimSheet.Editor.ToggleNames"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);

	Ref< ui::ToolBarMenu > menuExport = new ui::ToolBarMenu(i18n::Text(L"TRIMSHEET_EDITOR_EXPORT"), L"");
	menuExport->add(new ui::MenuItem(ui::Command(L"Render.TrimSheet.Editor.ExportLayer"), i18n::Text(L"TRIMSHEET_EDITOR_EXPORT_LAYER")));
	menuExport->add(new ui::MenuItem(ui::Command(L"Render.TrimSheet.Editor.ExportLayerGuides"), i18n::Text(L"TRIMSHEET_EDITOR_EXPORT_LAYER_GUIDES")));
	menuExport->add(new ui::MenuItem(ui::Command(L"Render.TrimSheet.Editor.ExportAllLayers"), i18n::Text(L"TRIMSHEET_EDITOR_EXPORT_ALL_LAYERS")));

	m_toolBar->addItem(m_toolLayer);
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TRIMSHEET_EDITOR_ADD_HORIZONTAL_SLAB"), ui::Command(L"Render.TrimSheet.Editor.AddHorizontalSlab")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TRIMSHEET_EDITOR_ADD_VERTICAL_SLAB"), ui::Command(L"Render.TrimSheet.Editor.AddVerticalSlab")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TRIMSHEET_EDITOR_SPLIT_REGION"), ui::Command(L"Render.TrimSheet.Editor.SplitRegion")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TRIMSHEET_EDITOR_REMOVE"), 0, ui::Command(L"Render.TrimSheet.Editor.Remove")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TRIMSHEET_EDITOR_MOVE_UP"), 1, ui::Command(L"Render.TrimSheet.Editor.MoveUp")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TRIMSHEET_EDITOR_MOVE_DOWN"), 2, ui::Command(L"Render.TrimSheet.Editor.MoveDown")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TRIMSHEET_EDITOR_ROTATE_IMAGE"), 3, ui::Command(L"Render.TrimSheet.Editor.RotateImage")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(m_toolToggleGuides);
	m_toolBar->addItem(m_toolToggleNames);
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TRIMSHEET_EDITOR_RELOAD"), ui::Command(L"Render.TrimSheet.Editor.Reload")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(menuExport);
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &TrimSheetSetupEditor::eventToolClick);

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(container, true, 220_ut);

	m_treeStructure = new ui::TreeView();
	m_treeStructure->create(splitter, ui::TreeView::WsTreeButtons | ui::TreeView::WsTreeLines | ui::WsDoubleBuffer | ui::WsTabStop);
	m_treeStructure->addEventHandler< ui::SelectionChangeEvent >(this, &TrimSheetSetupEditor::eventTreeSelect);

	m_control = new TrimSheetControl();
	m_control->create(splitter);
	m_control->addEventHandler< ui::SelectionChangeEvent >(this, &TrimSheetSetupEditor::eventControlSelect);
	m_control->addEventHandler< ui::ContentChangingEvent >(this, &TrimSheetSetupEditor::eventControlChanging);
	m_control->addEventHandler< ui::ContentChangeEvent >(this, &TrimSheetSetupEditor::eventControlChange);
	m_control->addEventHandler< ui::MouseButtonUpEvent >(this, &TrimSheetSetupEditor::eventControlMouseUp);
	m_control->addEventHandler< ui::MouseMoveEvent >(this, &TrimSheetSetupEditor::eventControlMouseMove);

	m_statusBar = new ui::StatusBar();
	m_statusBar->create(container);
	m_statusBar->addColumn(m_statusBar->pixel(260_ut));
	m_statusBar->addColumn(m_statusBar->pixel(360_ut));
	m_statusBar->addColumn(-1);

	m_propertiesView = m_site->createPropertiesView(parent);
	m_propertiesView->addEventHandler< ui::ContentChangingEvent >(this, &TrimSheetSetupEditor::eventPropertiesChanging);
	m_propertiesView->addEventHandler< ui::ContentChangeEvent >(this, &TrimSheetSetupEditor::eventPropertiesChanged);
	m_site->createAdditionalPanel(m_propertiesView, 400_ut, false);

	const bool showGuides = m_editor->getSettings()->getProperty< bool >(L"TrimSheetEditor.ShowGuides", true);
	const bool showNames = m_editor->getSettings()->getProperty< bool >(L"TrimSheetEditor.ShowNames", true);
	m_toolToggleGuides->setToggled(showGuides);
	m_toolToggleNames->setToggled(showNames);
	m_control->setShowGuides(showGuides);
	m_control->setShowNames(showNames);

	updateTree();
	updateSheet();
	updateSelection(true);
	return true;
}

void TrimSheetSetupEditor::destroy()
{
	if (m_toolToggleGuides && m_toolToggleNames)
	{
		Ref< PropertyGroup > settings = m_editor->checkoutGlobalSettings();
		if (settings)
		{
			settings->setProperty< PropertyBoolean >(L"TrimSheetEditor.ShowGuides", m_toolToggleGuides->isToggled());
			settings->setProperty< PropertyBoolean >(L"TrimSheetEditor.ShowNames", m_toolToggleNames->isToggled());
			m_editor->commitGlobalSettings();
		}
	}

	if (m_propertiesView)
		m_site->destroyAdditionalPanel(m_propertiesView);

	safeDestroy(m_propertiesView);

	m_composer = nullptr;
	m_sheet = nullptr;
	m_asset = nullptr;
	m_site = nullptr;
}

bool TrimSheetSetupEditor::dropInstance(db::Instance* instance, const ui::Point& position)
{
	// Dropping a texture asset on a region place its image in the current layer.
	Ref< TextureAsset > textureAsset = instance->getObject< TextureAsset >();
	if (!textureAsset)
		return false;

	// Sheet can extend outside of control when zoomed in, only accept drops over the control.
	const ui::Point clientPosition = m_control->screenToClient(position);
	if (!m_control->getInnerRect().inside(clientPosition))
		return false;

	int32_t slab, region;
	if (!m_control->hitRegion(clientPosition, slab, region))
		return false;

	m_document->push();

	TrimSheetRegion* dropRegion = m_asset->getRegion(slab, region);
	dropRegion->setFileName(m_layer, textureAsset->getFileName());
	makeRelativePaths(dropRegion);

	m_selectedSlab = slab;
	m_selectedRegion = region;

	updateTree();
	updateSheet();
	updateSelection(true);
	return true;
}

bool TrimSheetSetupEditor::handleCommand(const ui::Command& command)
{
	if (m_propertiesView->handleCommand(command))
		return true;

	if (command == L"Render.TrimSheet.Editor.Layer")
	{
		m_layer = (TrimSheetLayer)clamp< int32_t >(m_toolLayer->getSelected(), 0, TrimSheetLayerCount - 1);
		updateSheet();
	}
	else if (command == L"Render.TrimSheet.Editor.AddHorizontalSlab")
		addSlab(TrimSheetSlab::Orientation::Horizontal);
	else if (command == L"Render.TrimSheet.Editor.AddVerticalSlab")
		addSlab(TrimSheetSlab::Orientation::Vertical);
	else if (command == L"Render.TrimSheet.Editor.SplitRegion")
		splitRegion();
	else if (command == L"Render.TrimSheet.Editor.Remove")
		removeSelected();
	else if (command == L"Render.TrimSheet.Editor.MoveUp")
		moveSelected(-1);
	else if (command == L"Render.TrimSheet.Editor.MoveDown")
		moveSelected(1);
	else if (command == L"Render.TrimSheet.Editor.RotateImage")
		rotateImage();
	else if (command == L"Render.TrimSheet.Editor.ToggleGuides")
		m_control->setShowGuides(m_toolToggleGuides->isToggled());
	else if (command == L"Render.TrimSheet.Editor.ToggleNames")
		m_control->setShowNames(m_toolToggleNames->isToggled());
	else if (command == L"Render.TrimSheet.Editor.Reload")
	{
		m_composer->flush();
		updateSheet();
	}
	else if (command == L"Render.TrimSheet.Editor.ExportLayer")
		exportLayer(false);
	else if (command == L"Render.TrimSheet.Editor.ExportLayerGuides")
		exportLayer(true);
	else if (command == L"Render.TrimSheet.Editor.ExportAllLayers")
		exportAllLayers();
	else if (command == L"Editor.Delete")
	{
		if (!m_control->hasFocus() && !m_treeStructure->containFocus())
			return false;
		removeSelected();
	}
	else if (command == L"Editor.Undo" || command == L"Editor.Redo")
	{
		const bool result = (command == L"Editor.Undo") ? m_document->undo() : m_document->redo();
		if (result)
		{
			m_asset = m_document->getObject< TrimSheetSetupAsset >(0);
			T_ASSERT(m_asset);

			validateSelection();
			updateTree();
			updateSheet();
			updateSelection(true);
		}
	}
	else
		return false;

	return true;
}

void TrimSheetSetupEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void TrimSheetSetupEditor::updateTree()
{
	AlignedVector< TrimSheetRect > slabRects;
	AlignedVector< TrimSheetSetupAsset::RegionLayout > regionLayouts;
	m_asset->calculateLayout(slabRects, regionLayouts);

	m_treeStructure->removeAllItems();

	Ref< ui::TreeViewItem > sheetItem = m_treeStructure->createItem(nullptr, i18n::Format(L"TRIMSHEET_EDITOR_SHEET", m_asset->getWidth(), m_asset->getHeight()), 0);
	sheetItem->setData(L"STRUCTURE", new StructureItemData(-1, -1));

	const RefArray< TrimSheetSlab >& slabs = m_asset->getSlabs();
	uint32_t layoutIndex = 0;

	for (int32_t i = 0; i < (int32_t)slabs.size(); ++i)
	{
		const TrimSheetSlab* slab = slabs[i];
		const bool horizontal = (slab->getOrientation() == TrimSheetSlab::Orientation::Horizontal);
		const int32_t thickness = horizontal ? slabRects[i].height : slabRects[i].width;

		Ref< ui::TreeViewItem > slabItem = m_treeStructure->createItem(
			sheetItem,
			i18n::Format(horizontal ? L"TRIMSHEET_EDITOR_SLAB_HORIZONTAL" : L"TRIMSHEET_EDITOR_SLAB_VERTICAL", i, thickness),
			0
		);
		slabItem->setData(L"STRUCTURE", new StructureItemData(i, -1));

		const RefArray< TrimSheetRegion >& regions = slab->getRegions();
		for (int32_t j = 0; j < (int32_t)regions.size(); ++j, ++layoutIndex)
		{
			const TrimSheetRect& rc = regionLayouts[layoutIndex].rect;
			const int32_t length = horizontal ? rc.width : rc.height;

			Ref< ui::TreeViewItem > regionItem = m_treeStructure->createItem(
				slabItem,
				getRegionName(regions[j], i, j) + L" (" + toString(length) + L")",
				0
			);
			regionItem->setData(L"STRUCTURE", new StructureItemData(i, j));
		}

		slabItem->expand();
	}

	sheetItem->expand();
}

void TrimSheetSetupEditor::updateSheet()
{
	m_sheet = m_composer->compose(m_asset, m_layer);
	m_control->setSheet(m_asset, m_layer, m_sheet);
	updateImageBounds();
	updateStatus();
}

void TrimSheetSetupEditor::updateSelection(bool updateProperties)
{
	validateSelection();

	m_control->setSelection(m_selectedSlab, m_selectedRegion);

	for (auto item : m_treeStructure->getItems(ui::TreeView::GfDescendants))
	{
		const StructureItemData* data = item->getData< StructureItemData >(L"STRUCTURE");
		if (data && data->getSlab() == m_selectedSlab && data->getRegion() == m_selectedRegion)
		{
			item->select();
			item->show();
			break;
		}
	}
	m_treeStructure->update();

	if (updateProperties)
	{
		TrimSheetRegion* region = m_asset->getRegion(m_selectedSlab, m_selectedRegion);
		if (region)
			m_propertiesView->setPropertyObject(region);
		else if (m_selectedSlab >= 0)
			m_propertiesView->setPropertyObject(m_asset->getSlabs()[m_selectedSlab]);
		else
			m_propertiesView->setPropertyObject(m_asset);
	}

	updateImageBounds();
	updateStatus();
}

void TrimSheetSetupEditor::updateImageBounds()
{
	const TrimSheetRegion* region = m_asset->getRegion(m_selectedSlab, m_selectedRegion);

	TrimSheetLayer layer;
	int32_t width, height;
	if (region && getPlacementLayer(region, m_layer, layer) && m_composer->getPlacedSize(region, layer, width, height))
	{
		for (const auto& regionLayout : m_control->getRegionLayouts())
		{
			if (regionLayout.slab == m_selectedSlab && regionLayout.region == m_selectedRegion)
			{
				const TrimSheetRect bounds = {
					regionLayout.rect.x + region->getOffsetX(),
					regionLayout.rect.y + region->getOffsetY(),
					width,
					height
				};
				m_control->setImageBounds(&bounds);
				return;
			}
		}
	}
	m_control->setImageBounds(nullptr);
}

void TrimSheetSetupEditor::updateStatus()
{
	m_statusBar->setText(0, i18n::Format(L"TRIMSHEET_EDITOR_STATUS_SHEET", m_asset->getWidth(), m_asset->getHeight(), std::wstring(i18n::Text(c_layerTextIds[(int32_t)m_layer]))));

	const TrimSheetRegion* region = m_asset->getRegion(m_selectedSlab, m_selectedRegion);

	TrimSheetLayer layer;
	if (region && getPlacementLayer(region, m_layer, layer))
	{
		int32_t width, height;
		if (m_composer->getPlacedSize(region, layer, width, height))
			m_statusBar->setText(2, i18n::Format(L"TRIMSHEET_EDITOR_STATUS_IMAGE", width, height, region->getOffsetX(), region->getOffsetY()));
		else
			m_statusBar->setText(2, i18n::Format(L"TRIMSHEET_EDITOR_STATUS_IMAGE_MISSING", region->getFileName(layer).getOriginal()));
	}
	else
		m_statusBar->setText(2, L"");

	m_statusBar->update();
}

void TrimSheetSetupEditor::validateSelection()
{
	const RefArray< TrimSheetSlab >& slabs = m_asset->getSlabs();
	if (m_selectedSlab < 0 || m_selectedSlab >= (int32_t)slabs.size())
	{
		m_selectedSlab = -1;
		m_selectedRegion = -1;
		return;
	}

	const int32_t regionCount = (int32_t)slabs[m_selectedSlab]->getRegions().size();
	m_selectedRegion = clamp< int32_t >(m_selectedRegion, -1, regionCount - 1);
}

bool TrimSheetSetupEditor::makeRelativePaths(TrimSheetRegion* region) const
{
	const Path assetPath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath));
	bool modified = false;

	for (int32_t i = 0; i < TrimSheetLayerCount; ++i)
	{
		const Path fileName = region->getFileName((TrimSheetLayer)i);
		if (fileName.empty() || fileName.isRelative())
			continue;

		// Paths using environment variables are portable already.
		if (fileName.getOriginal().find(L"$(") != std::wstring::npos)
			continue;

		Path relativePath;
		if (!FileSystem::getInstance().getRelativePath(fileName, assetPath, relativePath))
			continue;

		// Keep absolute path if file is outside of asset path.
		if (startsWith(relativePath.getOriginal(), L".."))
			continue;

		region->setFileName((TrimSheetLayer)i, relativePath);
		modified = true;
	}

	return modified;
}

void TrimSheetSetupEditor::addSlab(TrimSheetSlab::Orientation orientation)
{
	m_document->push();

	const bool horizontal = (orientation == TrimSheetSlab::Orientation::Horizontal);
	const int32_t size = std::max((horizontal ? m_asset->getHeight() : m_asset->getWidth()) / 8, 1);

	Ref< TrimSheetSlab > slab = new TrimSheetSlab(orientation, size);
	slab->insertRegion(0, new TrimSheetRegion());

	// Insert after selected slab, or last.
	const int32_t index = (m_selectedSlab >= 0) ? m_selectedSlab + 1 : (int32_t)m_asset->getSlabs().size();
	m_asset->insertSlab(index, slab);

	m_selectedSlab = index;
	m_selectedRegion = 0;

	updateTree();
	updateSheet();
	updateSelection(true);
}

void TrimSheetSetupEditor::splitRegion()
{
	if (m_selectedSlab < 0)
		return;

	m_document->push();

	TrimSheetSlab* slab = m_asset->getSlabs()[m_selectedSlab];
	Ref< TrimSheetRegion > newRegion = new TrimSheetRegion();

	int32_t index = (int32_t)slab->getRegions().size();
	if (m_selectedRegion >= 0)
	{
		// Split fixed length region into two halves, auto length regions remain auto.
		TrimSheetRegion* region = slab->getRegions()[m_selectedRegion];
		if (region->getSize() > 1)
		{
			const int32_t half = region->getSize() / 2;
			newRegion->setSize(region->getSize() - half);
			region->setSize(half);
		}
		index = m_selectedRegion + 1;
	}

	slab->insertRegion(index, newRegion);
	m_selectedRegion = index;

	updateTree();
	updateSheet();
	updateSelection(true);
}

void TrimSheetSetupEditor::removeSelected()
{
	if (m_selectedSlab < 0)
		return;

	m_document->push();

	TrimSheetSlab* slab = m_asset->getSlabs()[m_selectedSlab];
	if (m_selectedRegion >= 0 && slab->getRegions().size() > 1)
	{
		// Remove region, select neighbour region.
		slab->removeRegion(m_selectedRegion);
		m_selectedRegion = std::min(m_selectedRegion, (int32_t)slab->getRegions().size() - 1);
	}
	else
	{
		// Remove entire slab, select previous slab.
		m_asset->removeSlab(m_selectedSlab);
		m_selectedSlab = std::min(m_selectedSlab, (int32_t)m_asset->getSlabs().size() - 1);
		m_selectedRegion = -1;
	}

	updateTree();
	updateSheet();
	updateSelection(true);
}

void TrimSheetSetupEditor::moveSelected(int32_t direction)
{
	if (m_selectedSlab < 0)
		return;

	RefArray< TrimSheetSlab >& slabs = m_asset->getSlabs();
	if (m_selectedRegion >= 0)
	{
		// Move region within its slab.
		RefArray< TrimSheetRegion >& regions = slabs[m_selectedSlab]->getRegions();
		const int32_t target = m_selectedRegion + direction;
		if (target < 0 || target >= (int32_t)regions.size())
			return;

		m_document->push();

		Ref< TrimSheetRegion > region = regions[m_selectedRegion];
		regions[m_selectedRegion] = regions[target];
		regions[target] = region;
		m_selectedRegion = target;
	}
	else
	{
		const int32_t target = m_selectedSlab + direction;
		if (target < 0 || target >= (int32_t)slabs.size())
			return;

		m_document->push();

		Ref< TrimSheetSlab > slab = slabs[m_selectedSlab];
		slabs[m_selectedSlab] = slabs[target];
		slabs[target] = slab;
		m_selectedSlab = target;
	}

	updateTree();
	updateSheet();
	updateSelection(true);
}

void TrimSheetSetupEditor::rotateImage()
{
	TrimSheetRegion* region = m_asset->getRegion(m_selectedSlab, m_selectedRegion);
	if (!region || !region->hasImage())
		return;

	m_document->push();
	region->setRotation((TrimSheetRegion::Rotation)(((int32_t)region->getRotation() + 1) % 4));

	updateSheet();
	updateSelection(true);
}

void TrimSheetSetupEditor::exportLayer(bool guides)
{
	if (!m_sheet)
		return;

	const std::wstring name = m_document->getInstance(0)->getName() + L" - " + c_layerNames[(int32_t)m_layer] + L".png";
	Path fileName = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath), Path(name));

	ui::FileDialog fileDialog;
	if (!fileDialog.create(m_control, type_name(this), i18n::Text(L"TRIMSHEET_EDITOR_EXPORT_TITLE"), L"PNG images;*.png", m_assetPath, true))
		return;
	const ui::DialogResult result = fileDialog.showModalThenDestroy(fileName);
	if (result != ui::DialogResult::Ok)
		return;

	if (fileName.getExtension().empty())
		fileName = Path(fileName.getPathName() + L".png");

	Ref< drawing::Image > image = m_sheet->clone();
	if (guides)
		drawGuides(image, m_asset);

	saveImage(image, fileName);
}

void TrimSheetSetupEditor::exportAllLayers()
{
	const std::wstring name = m_document->getInstance(0)->getName() + L".png";
	Path fileName = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath), Path(name));

	ui::FileDialog fileDialog;
	if (!fileDialog.create(m_control, type_name(this), i18n::Text(L"TRIMSHEET_EDITOR_EXPORT_TITLE"), L"PNG images;*.png", m_assetPath, true))
		return;
	const ui::DialogResult result = fileDialog.showModalThenDestroy(fileName);
	if (result != ui::DialogResult::Ok)
		return;

	// Each layer is saved as "<name> - <layer>.<extension>"; layers without any image are skipped.
	const std::wstring extension = !fileName.getExtension().empty() ? fileName.getExtension() : L"png";
	const std::wstring baseName = fileName.getPathOnly() + L"/" + fileName.getFileNameNoExtension();

	for (int32_t i = 0; i < TrimSheetLayerCount; ++i)
	{
		std::set< std::wstring > files;
		TrimSheetComposer::collectFiles(m_asset, (TrimSheetLayer)i, files);
		if (files.empty())
			continue;

		Ref< drawing::Image > image = m_composer->compose(m_asset, (TrimSheetLayer)i);
		if (image)
			saveImage(image, Path(baseName + L" - " + c_layerNames[i] + L"." + extension));
	}
}

void TrimSheetSetupEditor::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void TrimSheetSetupEditor::eventTreeSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::TreeViewItem > selectedItems = m_treeStructure->getItems(ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);
	if (selectedItems.size() != 1)
		return;

	const StructureItemData* data = selectedItems.front()->getData< StructureItemData >(L"STRUCTURE");
	if (!data)
		return;

	m_selectedSlab = data->getSlab();
	m_selectedRegion = data->getRegion();
	updateSelection(true);
}

void TrimSheetSetupEditor::eventControlSelect(ui::SelectionChangeEvent* event)
{
	m_selectedSlab = m_control->getSelectedSlab();
	m_selectedRegion = m_control->getSelectedRegion();
	updateSelection(true);
}

void TrimSheetSetupEditor::eventControlChanging(ui::ContentChangingEvent* event)
{
	m_document->push();
}

void TrimSheetSetupEditor::eventControlChange(ui::ContentChangeEvent* event)
{
	int32_t value = 0;
	const TrimSheetControl::DragMode dragMode = m_control->getDragMode(&value);

	if (dragMode == TrimSheetControl::DragMode::Image)
	{
		// Only dragged region changes; recompose it alone to stay interactive.
		for (const auto& regionLayout : m_control->getRegionLayouts())
		{
			if (regionLayout.slab == m_selectedSlab && regionLayout.region == m_selectedRegion)
			{
				m_composer->composeRegion(m_asset, m_layer, regionLayout, m_sheet);
				m_control->updateSheet(m_sheet, regionLayout.rect);
				break;
			}
		}
		updateImageBounds();
		updateStatus();
	}
	else
	{
		// Layout changed; recompose entire sheet when drag is released.
		m_sheetDirty = true;
		m_statusBar->setText(2, i18n::Format(L"TRIMSHEET_EDITOR_STATUS_SIZE", value));
		m_statusBar->update();
	}

	m_propertiesDirty = true;
}

void TrimSheetSetupEditor::eventControlMouseUp(ui::MouseButtonUpEvent* event)
{
	if (m_sheetDirty)
	{
		m_sheetDirty = false;
		updateTree();
		updateSheet();
	}
	if (m_propertiesDirty)
	{
		m_propertiesDirty = false;
		updateSelection(true);
	}
}

void TrimSheetSetupEditor::eventControlMouseMove(ui::MouseMoveEvent* event)
{
	int32_t x, y;
	if (!m_control->clientToSheet(event->getPosition(), x, y))
	{
		m_statusBar->setText(1, L"");
		m_statusBar->update();
		return;
	}

	std::wstring text = str(L"%d, %d", x, y);

	int32_t slab, region;
	if (m_control->hitRegion(event->getPosition(), slab, region))
		text += L" - " + getRegionName(m_asset->getRegion(slab, region), slab, region);

	m_statusBar->setText(1, text);
	m_statusBar->update();
}

void TrimSheetSetupEditor::eventPropertiesChanging(ui::ContentChangingEvent* event)
{
	m_document->push();
}

void TrimSheetSetupEditor::eventPropertiesChanged(ui::ContentChangeEvent* event)
{
	// Property list isn't rebound here as we're called from within it; items
	// show absolute paths until next time selection is updated.
	TrimSheetRegion* region = m_asset->getRegion(m_selectedSlab, m_selectedRegion);
	if (region)
		makeRelativePaths(region);

	updateTree();
	updateSheet();
	updateSelection(false);
}

}
