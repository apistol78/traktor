/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Clipboard.h"
#include "Ui/FileDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MessageBox.h"
#include "Ui/ShortcutTable.h"
#include "Ui/Splitter.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/TableLayout.h"
#include "Ui/ColorPicker/ColorDialog.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridHeader.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
#include "Ui/Theme/App/PreviewWidgetFactory.h"
#include "Ui/Theme/App/ThemeForm.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarMenu.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewContentChangeEvent.h"
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/TreeView/TreeViewItemActivateEvent.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor::ui
{
	namespace
	{

template < typename ItemType >
class Wrapper : public Object
{
public:
	explicit Wrapper(ItemType* ptr)
	:	m_ptr(ptr)
	{
	}

	ItemType* unwrap() { return m_ptr; }

private:
	ItemType* m_ptr;
};

Ref< StyleSheet > loadStyleSheet(const Path& pathName, bool resolve)
{
	Ref< traktor::IStream > file = FileSystem::getInstance().open(pathName, File::FmRead);
	if (file)
	{
		Ref< StyleSheet > styleSheet = xml::XmlDeserializer(file, pathName.getPathName()).readObject< StyleSheet >();
		if (!styleSheet)
			return nullptr;

		if (resolve)
		{
			auto includes = styleSheet->getInclude();
			for (const auto& include : includes)
			{
				Ref< StyleSheet > includeStyleSheet = loadStyleSheet(include, true);
				if (!includeStyleSheet)
					return nullptr;

				styleSheet = includeStyleSheet->merge(styleSheet);
				if (!styleSheet)
					return nullptr;
			}
		}

		auto& entities = styleSheet->getEntities();
		std::sort(entities.begin(), entities.end(), [](const StyleSheet::Entity& lh, const StyleSheet::Entity& rh) {
			return lh.typeName < rh.typeName;
		});

		return styleSheet;
	}
	else
		return nullptr;
}

bool saveStyleSheet(const Path& pathName, const StyleSheet* styleSheet)
{
	Ref< traktor::IStream > file = FileSystem::getInstance().open(pathName, File::FmWrite);
	if (file)
		return xml::XmlSerializer(file).writeObject(styleSheet);
	else
		return false;
}

bool isStyleSheet(const TreeViewItem* item)
{
	return item->getData(L"STYLESHEET") != nullptr;
}

bool isEntity(const TreeViewItem* item)
{
	return item->getData(L"ENTITY") != nullptr;
}

bool isElement(const TreeViewItem* item)
{
	return !isStyleSheet(item) && !isEntity(item);
}

TreeViewItem* getEntity(TreeViewItem* item)
{
	if (isEntity(item))
		return item;

	if (isElement(item))
	{
		T_ASSERT(isEntity(item->getParent()));
		return item->getParent();
	}

	return nullptr;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ThemeForm", ThemeForm, Form)

bool ThemeForm::create(const CommandLine& cmdLine)
{
	std::wstring styleSheetName = L"$(TRAKTOR_HOME)/resources/runtime/themes/Light/StyleSheet.xss";
	if (cmdLine.hasOption(L"styleSheet"))
		styleSheetName = cmdLine.getOption(L"styleSheet").getString();

	// Load default style sheet.
	Ref< StyleSheet > styleSheet = loadStyleSheet(styleSheetName, true);
	if (!styleSheet)
		return false;

	Application::getInstance()->setStyleSheet(styleSheet);

	if (!Form::create(L"Theme Editor", 1000_ut, 800_ut, Form::WsDefault, new TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)))
		return false;

	setIcon(new ui::StyleBitmap(L"ThemeEditor.Icon"));

	addEventHandler< ui::CloseEvent >(this, &ThemeForm::eventClose);

	// Create shortcut table.
	m_shortcutTable = new ui::ShortcutTable();
	m_shortcutTable->create();
	m_shortcutTable->addEventHandler< ui::ShortcutEvent >(this, &ThemeForm::eventShortcut);
	m_shortcutTable->addCommand(ui::KsCommand, ui::VkO, ui::Command(L"File.Open"));
	m_shortcutTable->addCommand(ui::KsCommand, ui::VkS, ui::Command(L"File.Save"));
	m_shortcutTable->addCommand(ui::KsCommand | ui::KsShift, ui::VkS, ui::Command(L"File.SaveAs"));
	m_shortcutTable->addCommand(ui::KsCommand, ui::VkX, ui::Command(L"Edit.Cut"));
	m_shortcutTable->addCommand(ui::KsCommand, ui::VkC, ui::Command(L"Edit.Copy"));
	m_shortcutTable->addCommand(ui::KsCommand, ui::VkV, ui::Command(L"Edit.Paste"));

	m_menuBar = new ToolBar();
	m_menuBar->create(this);
	m_menuBar->addEventHandler< ToolBarButtonClickEvent >(this, &ThemeForm::eventMenuClick);

	//m_menuItemMRU = new MenuItem(L"Recent");

	Ref< ToolBarMenu > menuFile = new ToolBarMenu(L"File", L"");
	menuFile->add(new MenuItem(Command(L"File.New"), L"New"));
	menuFile->add(new MenuItem(Command(L"File.Open"), L"Open..."));
	menuFile->add(new MenuItem(Command(L"File.Save"), L"Save"));
	menuFile->add(new MenuItem(Command(L"File.SaveAs"), L"Save As..."));
	//menuFile->add(m_menuItemMRU);
	menuFile->add(new MenuItem(L"-"));
	menuFile->add(new MenuItem(Command(L"File.Exit"), L"Exit"));
	m_menuBar->addItem(menuFile);

	Ref< ToolBarMenu > menuEdit = new ToolBarMenu(L"Edit", L"");
	menuEdit->add(new MenuItem(Command(L"Edit.Copy"), L"Copy"));
	menuEdit->add(new MenuItem(Command(L"Edit.Paste"), L"Paste"));
	m_menuBar->addItem(menuEdit);

	Ref< Splitter > splitter = new Splitter();
	splitter->create(this, true, 300_ut);

	Ref< Splitter > splitter2 = new Splitter();
	splitter2->create(splitter, false, -250_ut);

	m_treeTheme = new TreeView();
	m_treeTheme->create(
		splitter2,
		WsAccelerated |
		TreeView::WsAutoEdit |
		TreeView::WsTreeButtons |
		TreeView::WsTreeLines
	);
	m_treeTheme->addEventHandler< SelectionChangeEvent >(this, &ThemeForm::eventTreeSelectionChange);
	m_treeTheme->addEventHandler< TreeViewItemActivateEvent >(this, &ThemeForm::eventTreeActivateItem);
	m_treeTheme->addEventHandler< MouseButtonDownEvent >(this, &ThemeForm::eventTreeButtonDown);
	m_treeTheme->addEventHandler< TreeViewContentChangeEvent >(this, &ThemeForm::eventTreeChange);

	Ref< Tab > tab = new Tab();
	tab->create(splitter2, ui::WsNone);

	Ref< TabPage > tabPagePalette = new TabPage();
	tabPagePalette->create(tab, L"Palette", new FloodLayout());

	m_gridPalette = new GridView();
	m_gridPalette->create(tabPagePalette, WsDoubleBuffer | GridView::WsColumnHeader);
	m_gridPalette->addColumn(new GridColumn(L"Color", 70_ut));
	m_gridPalette->addColumn(new GridColumn(L"Hex", 150_ut));
	m_gridPalette->addEventHandler< MouseDoubleClickEvent >(this, &ThemeForm::eventPaletteDoubleClick);

	Ref< TabPage > tabPageValues = new TabPage();
	tabPageValues->create(tab, L"Values", new FloodLayout());

	m_gridValues = new GridView();
	m_gridValues->create(tabPageValues, WsDoubleBuffer | GridView::WsColumnHeader);
	m_gridValues->addColumn(new GridColumn(L"ID", 70_ut));
	m_gridValues->addColumn(new GridColumn(L"Value", 150_ut));

	tab->addPage(tabPagePalette);
	tab->addPage(tabPageValues);
	tab->setActivePage(tabPagePalette);

	m_containerPreview = new Container();
	m_containerPreview->create(splitter, WsAccelerated, new TableLayout(L"100%", L"100%", 16_ut, 0_ut));

	update();
	show();

	if (cmdLine.getCount() > 0)
	{
		if ((m_styleSheet = loadStyleSheet(cmdLine.getString(0), false)) != nullptr)
		{
			m_styleSheetPath = cmdLine.getString(0);
			m_styleSheetHash = DeepHash(m_styleSheet).get();
			updateTitle();
			updateTree();
			updatePalette();
			updateValues();
			updatePreview();
		}
		else
			ui::MessageBox::show(this, L"Unable to open stylesheet.", L"Error", ui::MbIconExclamation | ui::MbOk);
	}

	return true;
}

void ThemeForm::updateTree()
{
	auto state = m_treeTheme->captureState();

	m_treeTheme->removeAllItems();
	if (m_styleSheet)
	{
		Ref< TreeViewItem > itemStyleSheet = m_treeTheme->createItem(nullptr, L"Entities", 0);
		itemStyleSheet->setData(L"STYLESHEET", m_styleSheet);

		for (auto& entity : m_styleSheet->getEntities())
		{
			Ref< TreeViewItem > itemEntity = m_treeTheme->createItem(itemStyleSheet, entity.typeName, 0);
			itemEntity->setData(L"ENTITY", new Wrapper< StyleSheet::Entity >(&entity));

			for (const auto& it : entity.colors)
			{
				Ref< drawing::Image > imageColor = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), 16, 16);
				imageColor->clear(Color4f::fromColor4ub(it.second).rgb1());

				const int32_t imageIndex = m_treeTheme->addImage(new Bitmap(imageColor));

				Ref< TreeViewItem > itemElement = m_treeTheme->createItem(itemEntity, it.first, 1);
				itemElement->setImage(0, imageIndex);
			}
		}
	}

	m_treeTheme->applyState(state);
}

void ThemeForm::updatePalette()
{
	m_gridPalette->removeAllRows();

	if (!m_styleSheet)
		return;

	SmallSet< Color4ub > palette;
	for (const auto& entity : m_styleSheet->getEntities())
	{
		for (auto it : entity.colors)
			palette.insert(it.second);
	}

	for (const auto& color : palette)
	{
		Ref< drawing::Image > imageColor = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), 16, 16);
		imageColor->clear(Color4f::fromColor4ub(color).rgb1());

		Ref< GridRow > row = new GridRow();
		row->setData(L"COLOR", new PropertyColor(color));
		row->add(new Bitmap(imageColor));
		row->add(str(L"#%02x%02x%02x%02x", color.r, color.g, color.b, color.a));
		m_gridPalette->addRow(row);
	}
}

void ThemeForm::updateValues()
{
	m_gridValues->removeAllRows();

	if (!m_styleSheet)
		return;

	for (const auto& it : m_styleSheet->getValues())
	{
		Ref< GridRow > row = new GridRow();
		row->add(it.first);
		row->add(it.second);
		m_gridValues->addRow(row);
	}
}

void ThemeForm::updatePreview()
{
	while (m_containerPreview->getLastChild())
		m_containerPreview->getLastChild()->destroy();

	TreeViewItem* selectedEntityItem = nullptr;

	if (!m_pinnedElementItem)
	{
		RefArray< TreeViewItem > selectedItems;
		if (m_treeTheme->getItems(selectedItems, TreeView::GfDescendants | TreeView::GfSelectedOnly) != 1)
			return;
		selectedEntityItem = getEntity(selectedItems.front());
	}
	else
		selectedEntityItem = m_pinnedElementItem;

	if (!selectedEntityItem)
		return;

	auto entity = static_cast< Wrapper< StyleSheet::Entity >* >( selectedEntityItem->getData(L"ENTITY") )->unwrap();

	Ref< Container > container = new Container();
	container->create(m_containerPreview, 0, new TableLayout(L"100%", L"100%,100%", 0_ut, 4_ut));

	Ref< Widget > widget1 = PreviewWidgetFactory().create(container, m_styleSheet, entity->typeName);
	if (widget1)
		widget1->setEnable(true);

	Ref< Widget > widget2 = PreviewWidgetFactory().create(container, m_styleSheet, entity->typeName);
	if (widget2)
		widget2->setEnable(false);

	m_containerPreview->update();
}

void ThemeForm::updateTitle()
{
	if (m_styleSheet)
	{
		if (checkModified())
			setText(L"Theme Editor - " + m_styleSheetPath.getPathName() + L"*");
		else
			setText(L"Theme Editor - " + m_styleSheetPath.getPathName());
	}
	else
		setText(L"Theme Editor");
}

bool ThemeForm::checkModified() const
{
	if (m_styleSheet)
		return (m_styleSheetHash != DeepHash(m_styleSheet).get());
	else
		return false;
}

void ThemeForm::handleCommand(const Command& command)
{
	if (command == L"File.Open")
	{
		FileDialog fileDialog;
		fileDialog.create(this, type_name(this), L"Open stylesheet", L"Stylesheet;*.xss;All files;*.*");

		Path filePath;
		if (fileDialog.showModal(filePath) == ui::DialogResult::Ok)
		{
			if ((m_styleSheet = loadStyleSheet(filePath, false)) != nullptr)
			{
				m_styleSheetPath = filePath;
				m_styleSheetHash = DeepHash(m_styleSheet).get();
				updateTitle();
				updateTree();
				updatePalette();
				updateValues();
				updatePreview();
			}
			else
				ui::MessageBox::show(this, L"Unable to open stylesheet.", L"Error", ui::MbIconExclamation | ui::MbOk);
		}

		fileDialog.destroy();
	}
	else if (command == L"File.Save")
	{
		if (m_styleSheet != nullptr)
		{
			if (saveStyleSheet(m_styleSheetPath, m_styleSheet))
			{
				m_styleSheetHash = DeepHash(m_styleSheet).get();
				updateTitle();
			}
			else
				ui::MessageBox::show(this, L"Unable to save stylesheet.", L"Error", ui::MbIconExclamation | ui::MbOk);
		}
	}
	else if (command == L"File.SaveAs")
	{
		if (m_styleSheet != nullptr)
		{
			FileDialog fileDialog;
			fileDialog.create(this, type_name(this), L"Save stylesheet", L"Stylesheet;*.xss;All files;*.*", L"", true);

			Path filePath;
			if (fileDialog.showModal(filePath) == ui::DialogResult::Ok)
			{
				if (saveStyleSheet(filePath, m_styleSheet))
				{
					m_styleSheetPath = filePath;
					m_styleSheetHash = DeepHash(m_styleSheet).get();
					updateTitle();
				}
				else
					ui::MessageBox::show(this, L"Unable to save stylesheet.", L"Error", ui::MbIconExclamation | ui::MbOk);
			}

			fileDialog.destroy();
		}
	}
	else if (command == L"File.Exit")
		ui::Application::getInstance()->exit(0);
	else if (command == L"Edit.Copy")
	{
		RefArray< TreeViewItem > selectedItems;
		if (m_treeTheme->getItems(selectedItems, TreeView::GfDescendants | TreeView::GfSelectedOnly) != 1)
			return;

		Ref< TreeViewItem > selectedItem = selectedItems.front();
		if (isElement(selectedItem))
		{
			// Copy element.
			const Color4ub color = m_styleSheet->getColor(
				selectedItem->getParent()->getText(),
				selectedItem->getText()
			);

			Ref< PropertyGroup > props = new PropertyGroup();
			props->setProperty< PropertyString >(L"element", selectedItem->getText());
			props->setProperty< PropertyColor >(L"color", color);
			Application::getInstance()->getClipboard()->setObject(props);
		}
	}
	else if (command == L"Edit.Paste")
	{
		RefArray< TreeViewItem > selectedItems;
		if (m_treeTheme->getItems(selectedItems, TreeView::GfDescendants | TreeView::GfSelectedOnly) != 1)
			return;

		Ref< PropertyGroup > props = dynamic_type_cast< PropertyGroup* >(Application::getInstance()->getClipboard()->getObject());
		if (!props)
			return;

		Ref< TreeViewItem > selectedItem = selectedItems.front();
		if (isEntity(selectedItem))
		{
			const std::wstring element = props->getProperty< std::wstring >(L"element", L"");
			if (element.empty())
				return;

			const Color4ub color = props->getProperty< Color4ub >(L"color", Color4ub(255, 255, 255));

			m_styleSheet->setColor(
				selectedItem->getText(),
				element,
				color
			);

			updateTree();
			updatePreview();
			updateTitle();
		}	
	}
}

void ThemeForm::eventClose(CloseEvent*)
{
	ui::Application::getInstance()->exit(0);
}

void ThemeForm::eventShortcut(ShortcutEvent* event)
{
	handleCommand(event->getCommand());
}

void ThemeForm::eventMenuClick(ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void ThemeForm::eventTreeSelectionChange(SelectionChangeEvent* event)
{
	updatePreview();
}

void ThemeForm::eventTreeActivateItem(TreeViewItemActivateEvent* event)
{
	TreeViewItem* itemElement = event->getItem();
	if (!isElement(itemElement))
		return;

	TreeViewItem* itemEntity = getEntity(itemElement);
	T_ASSERT(itemEntity != nullptr);

	ColorDialog colorDialog;
	colorDialog.create(this, L"Set Element Color", Dialog::WsDefaultFixed | ColorDialog::WsAlpha);
	if (colorDialog.showModal() == ui::DialogResult::Ok)
	{
		const int32_t imageIndex = itemElement->getImage(0);

		Ref< drawing::Image > imageColor = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), 16, 16);
		imageColor->clear(colorDialog.getColor().rgb1());

		m_treeTheme->setImage(imageIndex, new Bitmap(imageColor));
		m_treeTheme->update();

		m_styleSheet->setColor(
			itemEntity->getText(),
			itemElement->getText(),
			colorDialog.getColor().toColor4ub()
		);

		updatePalette();
		updatePreview();
		updateTitle();
	}
	colorDialog.destroy();
}

void ThemeForm::eventTreeButtonDown(MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	RefArray< ui::TreeViewItem > selectedItems;
	m_treeTheme->getItems(selectedItems, TreeView::GfDescendants | TreeView::GfSelectedOnly);
	if (selectedItems.size() != 1)
		return;

	Ref< ui::TreeViewItem > selectedItem = selectedItems.front();
	if (!selectedItem)
		return;

	if (isStyleSheet(selectedItem)) // StyleSheet
	{
		Ref< Menu > menu = new Menu();
		menu->add(new MenuItem(Command(L"Theme.NewEntity"), L"Add New Entity"));

		const ui::MenuItem* menuItem = menu->showModal(m_treeTheme, event->getPosition());
		if (menuItem)
		{
			if (menuItem->getCommand() == L"Theme.NewEntity")
			{
				auto entity = m_styleSheet->addEntity(L"Unnamed");
				if (entity)
				{
					Ref< TreeViewItem > itemEntity = m_treeTheme->createItem(selectedItem, entity->typeName, 0);
					itemEntity->setData(L"ENTITY", new Wrapper< StyleSheet::Entity >(entity));
				}
			}
		}
	}
	else if (isEntity(selectedItem)) // Entity
	{
		Ref< Menu > menu = new Menu();
		menu->add(new MenuItem(Command(L"Theme.NewElement"), L"Add New Element"));
		if (selectedItem != m_pinnedElementItem)
			menu->add(new MenuItem(Command(L"Theme.PinElement"), L"Pin Element"));
		else
			menu->add(new MenuItem(Command(L"Theme.UnpinElement"), L"Unpin Element"));

		const ui::MenuItem* menuItem = menu->showModal(m_treeTheme, event->getPosition());
		if (menuItem)
		{
			if (menuItem->getCommand() == L"Theme.NewElement")
			{
				const Color4ub defaultColor(255, 255, 255);

				m_styleSheet->setColor(
					selectedItem->getText(),
					L"unnamed",
					defaultColor
				);

				Ref< drawing::Image > imageColor = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), 16, 16);
				imageColor->clear(Color4f::fromColor4ub(defaultColor).rgb1());

				const int32_t imageIndex = m_treeTheme->addImage(new Bitmap(imageColor));

				Ref< TreeViewItem > itemElement = m_treeTheme->createItem(selectedItem, L"unnamed", 1);
				itemElement->setImage(0, imageIndex);

				updatePalette();
			}
			else if (menuItem->getCommand() == L"Theme.PinElement")
			{
				if (m_pinnedElementItem)
					m_pinnedElementItem->setBold(false);
				m_pinnedElementItem = selectedItem;
				m_pinnedElementItem->setBold(true);
			}
			else if (menuItem->getCommand() == L"Theme.UnpinElement")
			{
				m_pinnedElementItem->setBold(false);
				m_pinnedElementItem = nullptr;
			}

			m_treeTheme->update();
		}
	}
	else if (isElement(selectedItem)) // Element
	{
	}

	updateTitle();
}

void ThemeForm::eventTreeChange(TreeViewContentChangeEvent* event)
{
	TreeViewItem* modifiedItem = event->getItem();

	if (isEntity(modifiedItem)) // Entity
	{
		auto entity = static_cast< Wrapper< StyleSheet::Entity >* >( modifiedItem->getData(L"ENTITY") )->unwrap();
		entity->typeName = modifiedItem->getText();
		updatePreview();
		event->consume();
	}
	else if (isElement(modifiedItem)) // Element
	{
		auto entityItem = getEntity(modifiedItem);
		T_ASSERT(entityItem != nullptr);

		auto entity = static_cast< Wrapper< StyleSheet::Entity >* >( entityItem->getData(L"ENTITY") )->unwrap();
		T_ASSERT(entity != nullptr);

		auto it = entity->colors.find(event->getOriginalText());
		T_ASSERT(it != entity->colors.end());
		Color4ub color = it->second;
		entity->colors.erase(it);

		if (!modifiedItem->getText().empty())
			entity->colors[modifiedItem->getText()] = color;

		updateTree();
		updatePreview();
		event->consume();
	}
}

void ThemeForm::eventPaletteDoubleClick(MouseDoubleClickEvent* event)
{
	auto selectedRow = m_gridPalette->getSelectedRow();
	if (selectedRow == nullptr)
		return;

	const Color4ub color = PropertyColor::get(selectedRow->getData< PropertyColor >(L"COLOR"));

	RefArray< ui::TreeViewItem > selectedItems;
	m_treeTheme->getItems(selectedItems, TreeView::GfDescendants | TreeView::GfSelectedOnly);
	if (selectedItems.size() != 1)
		return;

	auto itemElement = selectedItems.front();
	if (!isElement(itemElement))
		return;

	auto itemEntity = getEntity(itemElement);
	T_ASSERT(itemEntity);

	const int32_t imageIndex = itemElement->getImage(0);

	Ref< drawing::Image > imageColor = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), 16, 16);
	imageColor->clear(Color4f::fromColor4ub(color).rgb1());

	m_treeTheme->setImage(imageIndex, new Bitmap(imageColor));
	m_treeTheme->update();

	m_styleSheet->setColor(
		itemEntity->getText(),
		itemElement->getText(),
		color
	);

	updatePalette();
	updatePreview();
	updateTitle();
}

}
