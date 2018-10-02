/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/FloodLayout.h"
#include "Ui/TableLayout.h"
#include "Ui/FileDialog.h"
#include "Ui/Splitter.h"
#include "Ui/StyleBitmap.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowDoubleClickEvent.h"
#include "Ui/GridView/GridView.h"
#include "Ui/ListBox/ListBox.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.FileDialog", FileDialog, ConfigDialog)

bool FileDialog::create(Widget* parent, const std::wstring& title, const std::wstring& filters, bool save)
{
	if (!ConfigDialog::create(
		parent,
		title,
		dpi96(700),
		dpi96(500),
		ConfigDialog::WsDefaultResizable,
		new FloodLayout()
	))
		return false;

	Ref< Splitter > splitter = new Splitter();
	splitter->create(this, true, 15, true);

	Ref< ListBox > listFavorites = new ListBox();
	listFavorites->create(splitter);
	listFavorites->add(L"Home");
	listFavorites->add(L"Desktop");
	listFavorites->add(L"Downloads");

	Ref< Container > ct = new Container();
	ct->create(splitter, WsNone, new TableLayout(L"100%", L"*,100%", 0, 0));

	m_containerPath = new Container();
	m_containerPath->create(ct, WsNone, new TableLayout(L"*,*,*,*,*,*,*,*,*", L"100%", 0, dpi96(4)));

	m_gridFiles = new GridView();
	m_gridFiles->create(ct, GridView::WsColumnHeader | WsDoubleBuffer);
	m_gridFiles->addColumn(new GridColumn(L"", dpi96(20)));
	m_gridFiles->addColumn(new GridColumn(L"Filename", dpi96(300)));
	m_gridFiles->addColumn(new GridColumn(L"Size", dpi96(100)));
	m_gridFiles->addColumn(new GridColumn(L"Modified", dpi96(100)));
	m_gridFiles->setSortColumn(1, false, GridView::SmLexical);
	m_gridFiles->addEventHandler< GridRowDoubleClickEvent >([&](GridRowDoubleClickEvent* event) {

		auto file = event->getRow()->getData< File >(L"FILE");
		T_FATAL_ASSERT(file != nullptr);

		m_currentPath = file->getPath();

		if (file->isDirectory())
		{
			updatePath();
			updateFiles();			
		}
		else
		{
			endModal(DrOk);
		}
	});

	m_bitmapDirectory = new ui::StyleBitmap(L"UI.FileDialog.Directory");
	m_bitmapFile = new ui::StyleBitmap(L"UI.FileDialog.File");

	m_currentPath = FileSystem::getInstance().getCurrentVolumeAndDirectory();

	updatePath();
	updateFiles();

	return true;
}

void FileDialog::destroy()
{
	ConfigDialog::destroy();
}

int32_t FileDialog::showModal(Path& outPath)
{
	m_gridFiles->setMultiSelect(false);

	if (ConfigDialog::showModal() != DrOk)
		return DrCancel;

	auto selectedRow = m_gridFiles->getSelectedRow();
	if (selectedRow == nullptr)
		return DrCancel;

	auto file = selectedRow->getData< File >(L"FILE");
	T_FATAL_ASSERT(file != nullptr);

	if (file->isDirectory())
		return DrCancel;

	outPath = file->getPath();
	return DrOk;
}

int32_t FileDialog::showModal(std::vector< Path >& outPaths)
{
	m_gridFiles->setMultiSelect(true);

	if (ConfigDialog::showModal() != DrOk)
		return DrCancel;

	RefArray< ui::GridRow > rows;
	m_gridFiles->getRows(rows, ui::GridView::GfSelectedOnly);

	for (auto row : rows)
	{
		auto file = row->getData< File >(L"FILE");
		T_FATAL_ASSERT(file != nullptr);

		if (file->isDirectory())
			continue;

		outPaths.push_back(file->getPath());
	}

	return DrOk;
}

void FileDialog::updatePath()
{
	while (m_containerPath->getFirstChild() != nullptr)
		m_containerPath->getFirstChild()->destroy();

	auto pn = m_currentPath.getPathNameNoVolume();

	Path p;
	for (auto s : StringSplit< std::wstring >(pn, L"/"))
	{
		p = p + s;

		Ref< Button > buttonPath = new Button();
		buttonPath->create(m_containerPath, s);
		buttonPath->addEventHandler< ButtonClickEvent >([=](ButtonClickEvent* event) {
			m_currentPath = p;
			updatePath();
			updateFiles();
		});
	}

	m_containerPath->update();
}

void FileDialog::updateFiles()
{
	RefArray< File > files;
	FileSystem::getInstance().find(m_currentPath + L"*.*", files);

	m_gridFiles->removeAllRows();
	for (auto file : files)
	{
		auto fn = file->getPath().getFileName();
		if (fn == L"." || fn == L".." || file->isHidden())
			continue;

		Ref< GridRow > row = new GridRow();
		row->add(new GridItem(file->isDirectory() ? m_bitmapDirectory : m_bitmapFile));
		row->add(new GridItem(fn));

		if (!file->isDirectory())
			row->add(new GridItem(toString(file->getSize())));
		else
			row->add(new GridItem());

		if (!file->isDirectory())
			row->add(new GridItem(
				file->getLastWriteTime().format(L"%c")
			));
		else
			row->add(new GridItem());

		row->setData(L"FILE", file);

		m_gridFiles->addRow(row);
	}

	m_gridFiles->update();
}

	}
}
