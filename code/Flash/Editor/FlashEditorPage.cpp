#include "Flash/Editor/FlashEditorPage.h"
#include "Flash/Editor/FlashPreviewControl.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/SwfReader.h"
#include "Editor/IEditor.h"
#include "Editor/Settings.h"
#include "Ui/Container.h"
#include "Ui/Bitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/CenterLayout.h"
#include "Ui/Custom/AspectLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Database/Instance.h"
#include "Render/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"

// Resources
#include "Resources/Playback.h"
#include "Resources/Aspect.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashEditorPage", FlashEditorPage, editor::IEditorPage)

FlashEditorPage::FlashEditorPage(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool FlashEditorPage::create(ui::Container* parent, editor::IEditorPageSite* site)
{
	Ref< render::IRenderSystem > renderSystem = m_editor->getRenderSystem();
	Ref< db::Database > database = m_editor->getOutputDatabase();

	m_resourceManager = new resource::ResourceManager();
	m_resourceManager->addFactory(
		new render::ShaderFactory(database, renderSystem)
	);

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolBarPlay = new ui::custom::ToolBar();
	m_toolBarPlay->create(container);
	m_toolBarPlay->addImage(ui::Bitmap::load(c_ResourcePlayback, sizeof(c_ResourcePlayback), L"png"), 6);
	m_toolBarPlay->addImage(ui::Bitmap::load(c_ResourceAspect, sizeof(c_ResourceAspect), L"png"), 2);
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(L"Rewind", ui::Command(L"Flash.Editor.Rewind"), 0));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(L"Play", ui::Command(L"Flash.Editor.Play"), 1));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(L"Stop", ui::Command(L"Flash.Editor.Stop"), 2));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(L"Forward", ui::Command(L"Flash.Editor.Forward"), 3));
	m_toolBarPlay->addItem(new ui::custom::ToolBarSeparator());
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(L"Layout Center", ui::Command(L"Flash.Editor.LayoutCenter"), 6));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(L"Layout Aspect", ui::Command(L"Flash.Editor.LayoutAspect"), 7));
	m_toolBarPlay->addClickEventHandler(ui::createMethodHandler(this, &FlashEditorPage::eventToolClick));

	m_previewContainer = new ui::Container();
	m_previewContainer->create(container, ui::WsNone, new ui::custom::CenterLayout());

	m_previewControl = new FlashPreviewControl();
	m_previewControl->create(m_previewContainer, ui::WsNone, m_resourceManager, renderSystem);

	return true;
}

void FlashEditorPage::destroy()
{
	m_previewControl->destroy();
}

void FlashEditorPage::activate()
{
}

void FlashEditorPage::deactivate()
{
}

bool FlashEditorPage::setDataObject(db::Instance* instance, Object* data)
{
	m_movieInstance = instance;

	Ref< FlashMovieAsset > asset = checked_type_cast< FlashMovieAsset* >(data);

	std::wstring assetPath = m_editor->getSettings()->getProperty< editor::PropertyString >(L"Pipeline.AssetPath", L"");
	Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());
	Ref< IStream > stream = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!stream)
		return false;

	uint32_t assetSize = stream->available();
	std::vector< uint8_t > assetBlob(assetSize);

	uint32_t offset = 0;
	while (offset < assetSize)
	{
		int nread = stream->read(&assetBlob[offset], assetSize - offset);
		if (nread < 0)
			return false;
		offset += nread;
	}

	stream->close();

	Ref< MemoryStream > memoryStream = new MemoryStream(&assetBlob[0], int(assetSize), true, false);
	Ref< SwfReader > swf = new SwfReader(memoryStream);

	m_movie = flash::FlashMovieFactory().createMovie(swf);
	if (!m_movie)
		return false;

	m_previewControl->setMovie(m_movie);
	m_previewContainer->update();

	return true;
}

Ref< db::Instance > FlashEditorPage::getDataInstance()
{
	return m_movieInstance;
}

Ref< Object > FlashEditorPage::getDataObject()
{
	return 0;
}

bool FlashEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool FlashEditorPage::handleCommand(const ui::Command& command)
{
	bool result = true;

	if (command == L"Flash.Editor.Rewind")
		m_previewControl->rewind();
	else if (command == L"Flash.Editor.Play")
		m_previewControl->play();
	else if (command == L"Flash.Editor.Stop")
		m_previewControl->stop();
	else if (command == L"Flash.Editor.Forward")
		m_previewControl->forward();
	else if (command == L"Flash.Editor.LayoutCenter")
	{
		m_previewContainer->setLayout(new ui::custom::CenterLayout());
		m_previewContainer->update();
	}
	else if (command == L"Flash.Editor.LayoutAspect")
	{
		m_previewContainer->setLayout(new ui::custom::AspectLayout());
		m_previewContainer->update();
	}
	else
		result = false;

	return result;
}

void FlashEditorPage::handleDatabaseEvent(const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->update(eventId, true);
}

void FlashEditorPage::eventToolClick(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	handleCommand(command);
}

	}
}
