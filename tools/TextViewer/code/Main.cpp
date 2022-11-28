#if defined(_WIN32)
#	include <Windows.h>
#endif

#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Io/StringReader.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/Io/Utf8Encoding.h>
#include <Ui/Application.h>
#include <Ui/Form.h>
#include <Ui/TableLayout.h>
#include <Ui/Splitter.h>
#include <Ui/StyleSheet.h>
#include <Ui/RichEdit/RichEdit.h>
#include <Ui/FileDialog.h>
#include <Ui/ToolBar/ToolBar.h>
#include <Ui/ToolBar/ToolBarButton.h>
#include <Ui/ToolBar/ToolBarButtonClickEvent.h>
#include <Xml/XmlDeserializer.h>

#if defined(_WIN32)
#	include <Ui/Win32/WidgetFactoryWin32.h>
typedef traktor::ui::WidgetFactoryWin32 WidgetFactoryImpl;
#elif defined(__APPLE__)
#	include <Ui/Cocoa/WidgetFactoryCocoa.h>
typedef traktor::ui::WidgetFactoryCocoa WidgetFactoryImpl;
#elif defined(__GNUC__)
#	include <Ui/Gtk/WidgetFactoryGtk.h>
typedef traktor::ui::WidgetFactoryGtk WidgetFactoryImpl;
#else
#	include <Ui/Wx/WidgetFactoryWx.h>
typedef traktor::ui::WidgetFactoryWx WidgetFactoryImpl;
#endif

#if defined(MessageBox)
#	undef MessageBox
#endif

using namespace traktor;

Ref< ui::StyleSheet > loadStyleSheet(const Path& pathName, bool resolve)
{
	Ref< traktor::IStream > file = FileSystem::getInstance().open(pathName, File::FmRead);
	if (file)
	{
		Ref< ui::StyleSheet > styleSheet = xml::XmlDeserializer(file, pathName.getPathName()).readObject< ui::StyleSheet >();
		if (!styleSheet)
			return nullptr;

		if (resolve)
		{
			auto includes = styleSheet->getInclude();
			for (const auto& include : includes)
			{
				Ref< ui::StyleSheet > includeStyleSheet = loadStyleSheet(include, true);
				if (!includeStyleSheet)
					return nullptr;

				styleSheet = includeStyleSheet->merge(styleSheet);
				if (!styleSheet)
					return nullptr;
			}
		}

		auto& entities = styleSheet->getEntities();
		std::sort(entities.begin(), entities.end(), [](const ui::StyleSheet::Entity& lh, const ui::StyleSheet::Entity& rh) {
			return lh.typeName < rh.typeName;
		});

		return styleSheet;
	}
	else
		return nullptr;
}

class MainForm : public ui::Form
{
public:
	bool create();

private:
	Ref< ui::Splitter > m_splitter;
	Ref< ui::RichEdit > m_editOld;
	Ref< ui::RichEdit > m_editCurrent;
	Path m_fileName;
	DateTime m_lastWriteTime;

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

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

	Ref< ui::ToolBar > toolBar = new ui::ToolBar();
	toolBar->create(this);
	toolBar->addItem(new ui::ToolBarButton(L"Select file...", ui::Command(L"TextViewer.SelectFile")));
	toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &MainForm::eventToolClick);

	m_splitter = new ui::Splitter();
	m_splitter->create(this, true, 50, true);

	m_editOld = new ui::RichEdit();
	m_editOld->create(m_splitter);
	m_editOld->setFont(ui::Font(L"Consolas", 14));
	m_editOld->addEventHandler< ui::FileDropEvent >(this, &MainForm::eventFileDrop);

	m_editCurrent = new ui::RichEdit();
	m_editCurrent->create(m_splitter);
	m_editCurrent->setFont(ui::Font(L"Consolas", 14));
	m_editCurrent->addEventHandler< ui::FileDropEvent >(this, &MainForm::eventFileDrop);

	addEventHandler< ui::TimerEvent >(this, &MainForm::eventTimer);

	update();
	show();

	startTimer(100);
	return true;
}

void MainForm::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	ui::FileDialog fileDialog;

	fileDialog.create(this, L"TextViewer", L"Select file...", L"All files;*.*", L"", false);
	if (fileDialog.showModal(m_fileName) == ui::DialogResult::Ok)
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
		new WidgetFactoryImpl(),
		nullptr
	);

	Ref< ui::StyleSheet > styleSheet = loadStyleSheet(L"$(TRAKTOR_HOME)/resources/runtime/themes/Light/StyleSheet.xss", true);
	if (!styleSheet)
		return 1;

	ui::Application::getInstance()->setStyleSheet(styleSheet);

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
