#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/SwfReader.h"
#include "Flash/Editor/FlashEditorPage.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Flash/Editor/FlashPreviewControl.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Sound/SoundSystem.h"
#include "Sound/Editor/SoundSystemFactory.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/CenterLayout.h"
#include "Ui/Custom/AspectLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

// Resources
#include "Resources/Playback.h"
#include "Resources/Aspect.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashEditorPage", FlashEditorPage, editor::IEditorPage)

FlashEditorPage::FlashEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool FlashEditorPage::create(ui::Container* parent)
{
	Ref< render::IRenderSystem > renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	Ref< sound::SoundSystemFactory > soundSystemFactory = m_editor->getStoreObject< sound::SoundSystemFactory >(L"SoundSystemFactory");
	if (soundSystemFactory)
		m_soundSystem = soundSystemFactory->createSoundSystem();

	Ref< FlashMovieAsset > asset = m_document->getObject< FlashMovieAsset >(0);
	if (!asset)
		return false;

	std::wstring assetPath = m_editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
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

	Ref< db::Database > database = m_editor->getOutputDatabase();

	m_resourceManager = new resource::ResourceManager(true);
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
	m_toolBarPlay->addClickEventHandler(ui::createMethodHandler(this, &FlashEditorPage::eventToolClick));

	m_previewControl = new FlashPreviewControl();
	m_previewControl->create(container, ui::WsNone, database, m_resourceManager, renderSystem, m_soundSystem);
	m_previewControl->setMovie(m_movie);
	m_previewControl->update();

	return true;
}

void FlashEditorPage::destroy()
{
	safeDestroy(m_previewControl);
	safeDestroy(m_soundSystem);
}

void FlashEditorPage::activate()
{
}

void FlashEditorPage::deactivate()
{
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
	else
		result = false;

	return result;
}

void FlashEditorPage::handleDatabaseEvent(const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->reload(eventId);
}

void FlashEditorPage::eventToolClick(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	handleCommand(command);
}

	}
}
