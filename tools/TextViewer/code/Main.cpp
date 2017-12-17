/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Io/StringReader.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/Io/Utf8Encoding.h>

#include <Ui/Application.h>
#include <Ui/FloodLayout.h>
#include <Ui/Form.h>
#include <Ui/TableLayout.h>
#include <Ui/Custom/Splitter.h>
#include <Ui/Custom/RichEdit/RichEdit.h>

//#include <Ui/Button.h>
//#include <Ui/CheckBox.h>
//#include <Ui/DropDown.h>
#include <Ui/FileDialog.h>
//#include <Ui/ListBox.h>
//#include <Ui/ListView.h>
//#include <Ui/MenuBar.h>
//#include <Ui/MenuItem.h>
//#include <Ui/MessageBox.h>
//#include <Ui/ScrollBar.h>
//#include <Ui/Static.h>
//#include <Ui/ToolForm.h>
//#include <Ui/TreeView.h>
//#include <Ui/TreeViewItem.h>
//#include <Ui/Container.h>
//#include <Ui/Events/CommandEvent.h>
//#include <Ui/Custom/MiniButton.h>
//#include <Ui/Custom/Splitter.h>
#include <Ui/Custom/ToolBar/ToolBar.h>
#include <Ui/Custom/ToolBar/ToolBarButton.h>
#include <Ui/Custom/ToolBar/ToolBarButtonClickEvent.h>

//#include <Ui/Custom/GridView/GridView.h>
//#include <Ui/Custom/GridView/GridColumn.h>
//#include <Ui/Custom/GridView/GridRow.h>
//#include <Ui/Custom/GridView/GridItem.h>
//#include <Ui/Custom/Sequencer/SequencerControl.h>
//#include <Ui/Custom/Sequencer/Sequence.h>
//#include <Ui/Custom/Sequencer/Range.h>
//#include <Core/Log/Log.h>

#if defined(_WIN32)
#	include <Ui/Win32/EventLoopWin32.h>
#	include <Ui/Win32/WidgetFactoryWin32.h>
typedef traktor::ui::EventLoopWin32 EventLoopImpl;
typedef traktor::ui::WidgetFactoryWin32 WidgetFactoryImpl;
#elif defined(__APPLE__)
#	include <Ui/Cocoa/EventLoopCocoa.h>
#	include <Ui/Cocoa/WidgetFactoryCocoa.h>
typedef traktor::ui::EventLoopCocoa EventLoopImpl;
typedef traktor::ui::WidgetFactoryCocoa WidgetFactoryImpl;
#elif defined(__GNUC__)
#	include <Ui/Gtk/EventLoopGtk.h>
#	include <Ui/Gtk/WidgetFactoryGtk.h>
typedef traktor::ui::EventLoopGtk EventLoopImpl;
typedef traktor::ui::WidgetFactoryGtk WidgetFactoryImpl;
#else
#	include <Ui/Wx/EventLoopWx.h>
#	include <Ui/Wx/WidgetFactoryWx.h>
typedef traktor::ui::EventLoopWx EventLoopImpl;
typedef traktor::ui::WidgetFactoryWx WidgetFactoryImpl;
#endif

#if defined(MessageBox)
#	undef MessageBox
#endif

using namespace traktor;

class MainForm : public ui::Form
{
public:
	bool create();

private:
	Ref< ui::custom::Splitter > m_splitter;
	Ref< ui::custom::RichEdit > m_editOld;
	Ref< ui::custom::RichEdit > m_editCurrent;
	Path m_fileName;
	DateTime m_lastWriteTime;

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventFileDrop(ui::FileDropEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

bool MainForm::create()
{
	if (!ui::Form::create(
		L"Text Viewer",
		800,
		700,
		ui::Form::WsDefault,
		new ui::TableLayout(L"100%", L"*,100%", 0, 0)
	))
		return false;

	Ref< ui::custom::ToolBar > toolBar = new ui::custom::ToolBar();
	toolBar->create(this);
	toolBar->addItem(new ui::custom::ToolBarButton(L"Select file...", ui::Command(L"TextViewer.SelectFile")));
	toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &MainForm::eventToolClick);

	m_splitter = new ui::custom::Splitter();
	m_splitter->create(this, true, 50, true);

	m_editOld = new ui::custom::RichEdit();
	m_editOld->create(m_splitter);
	m_editOld->setFont(ui::Font(L"Consolas", 14));
	m_editOld->addEventHandler< ui::FileDropEvent >(this, &MainForm::eventFileDrop);

	m_editCurrent = new ui::custom::RichEdit();
	m_editCurrent->create(m_splitter);
	m_editCurrent->setFont(ui::Font(L"Consolas", 14));
	m_editCurrent->addEventHandler< ui::FileDropEvent >(this, &MainForm::eventFileDrop);

	addEventHandler< ui::TimerEvent >(this, &MainForm::eventTimer);

	update();
	show();

	startTimer(100);
	return true;
}

void MainForm::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	ui::FileDialog fileDialog;

	fileDialog.create(this, L"Select file...", L"All files;*.*", false);
	if (fileDialog.showModal(m_fileName) == ui::DrOk)
	{
		m_editOld->setText(L"");
		m_editCurrent->setText(L"");

		m_lastWriteTime = DateTime();
	}

	fileDialog.destroy();
}

void MainForm::eventFileDrop(ui::FileDropEvent* event)
{
	const std::vector< Path >& files = event->getFiles();
	if (!files.empty())
		m_fileName = files[0];
	else
		m_fileName = Path();

	m_editOld->setText(L"");
	m_editCurrent->setText(L"");

	m_lastWriteTime = DateTime();
}

void MainForm::eventTimer(ui::TimerEvent* event)
{
	if (m_fileName.empty())
		return;

	Ref< File > file = FileSystem::getInstance().get(m_fileName);
	if (file)
	{
		DateTime lastWriteTime = file->getLastWriteTime();
		if (lastWriteTime != m_lastWriteTime)
		{
			StringOutputStream ss;

			Ref< traktor::IStream > stream = FileSystem::getInstance().open(m_fileName, File::FmRead);
			if (stream)
			{
				Utf8Encoding utf8;
				StringReader reader(stream, &utf8);

				std::wstring line;
				while (reader.readLine(line) >= 0)
					ss << line << Endl;

				stream->close();
				stream = 0;

				if (!ss.empty())
				{
					int32_t offsetCurrent = m_editCurrent->getScrollLine();

					m_editOld->setText(m_editCurrent->getText());
					m_editOld->scrollToLine(offsetCurrent);
					m_editOld->update();

					m_editCurrent->setText(ss.str());
					m_editCurrent->scrollToLine(offsetCurrent);
					m_editCurrent->update();

					m_lastWriteTime = lastWriteTime;
				}
			}
		}
	}
}

#if defined(_WIN32)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main()
#endif
{
	ui::Application::getInstance()->initialize(
		new EventLoopImpl(),
		new WidgetFactoryImpl(),
		0
	);

	MainForm form;

	bool result = true;
	if ((result &= form.create()) == false)
		return 1;

	if (result)
	{
		ui::Application::getInstance()->execute();
		form.destroy();
	}

	ui::Application::getInstance()->finalize();
	return 0;
}
