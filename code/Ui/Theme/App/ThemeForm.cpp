#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Clipboard.h"
#include "Ui/FileDialog.h"
#include "Ui/MenuItem.h"
#include "Ui/MessageBox.h"
#include "Ui/Splitter.h"
#include "Ui/StyleSheet.h"
#include "Ui/TableLayout.h"
#include "Ui/ColorPicker/ColorDialog.h"
#include "Ui/Theme/App/PreviewWidgetFactory.h"
#include "Ui/Theme/App/ThemeForm.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarMenu.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/TreeView/TreeViewItemActivateEvent.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

Ref< StyleSheet > loadStyleSheet(const Path& pathName)
{
	Ref< traktor::IStream > file = FileSystem::getInstance().open(pathName, File::FmRead);
	if (file)
		return xml::XmlDeserializer(file, pathName.getPathName()).readObject< StyleSheet >();
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ThemeForm", ThemeForm, Form)

bool ThemeForm::create()
{
	// Load stylesheet.
	std::wstring styleSheetName = L"$(TRAKTOR_HOME)/resources/runtime/themes/Dark/StyleSheet.xss";
	Ref< StyleSheet > styleSheet = loadStyleSheet(styleSheetName);
	if (!styleSheet)
	{
		log::error << L"Unable to load stylesheet \"" << styleSheetName << L"\"." << Endl;
		return false;
	}
	Application::getInstance()->setStyleSheet(styleSheet);

	if (!Form::create(L"Theme Editor", 1000, 800, Form::WsDefault, new TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

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

	Ref< ToolBarMenu > menuTools = new ToolBarMenu(L"Tools", L"");
	//menuTools->add(new MenuItem(Command(L"Tools.Synchronize"), L"Synchronize..."));
	m_menuBar->addItem(menuTools);

	Ref< Splitter > splitter = new Splitter();
	splitter->create(this, true, dpi96(300));

	m_treeTheme = new TreeView();
	m_treeTheme->create(
		splitter,
		WsAccelerated |
		TreeView::WsAutoEdit |
		TreeView::WsTreeButtons |
		TreeView::WsTreeLines
	);
	m_treeTheme->addEventHandler< SelectionChangeEvent >(this, &ThemeForm::eventTreeSelectionChange);
	m_treeTheme->addEventHandler< TreeViewItemActivateEvent >(this, &ThemeForm::eventTreeActivateItem);

	m_containerPreview = new Container();
	m_containerPreview->create(splitter, WsAccelerated, new TableLayout(L"100%", L"100%", 16, 0));

	update();
	show();

	return true;
}

void ThemeForm::updateTree()
{
	auto state = m_treeTheme->captureState();

	m_treeTheme->removeAllItems();

	std::map< std::wstring, std::list< const StyleSheet::Group* > > gm;
	for (const auto& group : m_styleSheet->getGroups())
		gm[group.type].push_back(&group);

	for (const auto& it : gm)
	{
		Ref< TreeViewItem > itemWidget = m_treeTheme->createItem(nullptr, it.first, 0);
		for (const auto& g : it.second)
		{
			Ref< drawing::Image > imageColor = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), 16, 16);
			imageColor->clear(Color4f::fromColor4ub(g->color).rgb1());

			int32_t imageIndex = m_treeTheme->addImage(new Bitmap(imageColor), 1);

			Ref< TreeViewItem > itemElement = m_treeTheme->createItem(itemWidget, g->element, 1);
			itemElement->setImage(0, imageIndex);
		}
	}

	m_treeTheme->applyState(state);
}

void ThemeForm::updatePreview()
{
	while (m_containerPreview->getLastChild())
		m_containerPreview->getLastChild()->destroy();

	RefArray< TreeViewItem > selectedItems;
	if (m_treeTheme->getItems(selectedItems, TreeView::GfDescendants | TreeView::GfSelectedOnly) != 1)
		return;

	Ref< TreeViewItem > selectedItem = selectedItems.front();
	while (selectedItem->getParent())
		selectedItem = selectedItem->getParent();

	std::wstring widgetTypeName = selectedItem->getText();
	if (!widgetTypeName.empty())
	{
		Ref< Container > container = new Container();
		container->create(m_containerPreview, 0, new TableLayout(L"100%", L"100%,100%", 0, 4));

		Ref< Widget > widget1 = PreviewWidgetFactory().create(container, m_styleSheet, widgetTypeName);
		if (widget1)
			widget1->setEnable(true);

		Ref< Widget > widget2 = PreviewWidgetFactory().create(container, m_styleSheet, widgetTypeName);
		if (widget2)
			widget2->setEnable(false);
	}

	m_containerPreview->update();
}

void ThemeForm::handleCommand(const Command& command)
{
	if (command == L"File.Open")
	{
		FileDialog fileDialog;
		fileDialog.create(this, type_name(this), L"Open stylesheet", L"Stylesheet;*.xss;All files;*.*");

		Path filePath;
		if (fileDialog.showModal(filePath))
		{
			if ((m_styleSheet = loadStyleSheet(filePath)) != nullptr)
			{
				m_styleSheetPath = filePath;
				updateTree();
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
			if (!saveStyleSheet(m_styleSheetPath, m_styleSheet))
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
			if (fileDialog.showModal(filePath))
			{
				if (saveStyleSheet(filePath, m_styleSheet))
					m_styleSheetPath = filePath;
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
		if (selectedItem->getParent() != nullptr)
		{
			// Copy element.
			Color4ub color = m_styleSheet->getColor(
				selectedItem->getParent()->getText(),
				selectedItem->getText()
			);

			Ref< PropertyGroup > props = new PropertyGroup();
			props->setProperty< PropertyString >(L"element", selectedItem->getText());
			props->setProperty< PropertyColor >(L"color", color);
			Application::getInstance()->getClipboard()->setObject(props);
		}
		else
		{
			// Copy widget.
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

		std::wstring element = props->getProperty< std::wstring >(L"element", L"");
		if (element.empty())
			return;

		Color4ub color = props->getProperty< Color4ub >(L"color", Color4ub(255, 255, 255));

		Ref< TreeViewItem > selectedItem = selectedItems.front();
		if (selectedItem->getParent() == nullptr)
		{
			//int32_t imageIndex = selectedItem->getImage(0);

			//Ref< drawing::Image > imageColor = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), 16, 16);
			//imageColor->clear(Color4f::fromColor4ub(color).rgb1());

			//m_treeTheme->setImage(imageIndex, new Bitmap(imageColor));
			//m_treeTheme->update();

			m_styleSheet->setColor(
				selectedItem->getText(),
				element,
				color
			);

			updatePreview();
		}
	}
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
	Ref< TreeViewItem > item = event->getItem();
	if (item->getParent() == nullptr)
		return;

	ColorDialog colorDialog;
	colorDialog.create(this, L"Set element color");
	if (colorDialog.showModal() == DialogResult::DrOk)
	{
		int32_t imageIndex = item->getImage(0);

		Ref< drawing::Image > imageColor = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), 16, 16);
		imageColor->clear(colorDialog.getColor().rgb1());

		m_treeTheme->setImage(imageIndex, new Bitmap(imageColor));
		m_treeTheme->update();

		m_styleSheet->setColor(
			item->getParent()->getText(),
			item->getText(),
			colorDialog.getColor().toColor4ub()
		);

		updatePreview();
	}
	colorDialog.destroy();
}

	}
}