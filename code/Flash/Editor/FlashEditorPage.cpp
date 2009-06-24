#include "Flash/Editor/FlashEditorPage.h"
#include "Flash/Editor/FlashPreviewControl.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/SwfReader.h"
#include "Editor/IEditor.h"
#include "Editor/IProject.h"
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
#include "Resource/ResourceCache.h"
#include "Resource/ResourceLoader.h"
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

bool FlashEditorPage::create(ui::Container* parent)
{
	Ref< render::RenderSystem > renderSystem = m_editor->getRenderSystem();
	Ref< editor::IProject > project = m_editor->getProject();
	Ref< db::Database > database = project->getOutputDatabase();

	m_resourceCache = gc_new< resource::ResourceCache >();
	m_resourceLoader = gc_new< resource::ResourceLoader >();

	m_resourceLoader->addFactory(
		gc_new< render::ShaderFactory >(database, renderSystem)
	);

	resource::ResourceManager::getInstance().setCache(m_resourceCache);
	resource::ResourceManager::getInstance().addLoader(m_resourceLoader);

	Ref< ui::Container > container = gc_new< ui::Container >();
	container->create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"*,100%", 0, 0));

	m_toolBarPlay = gc_new< ui::custom::ToolBar >();
	m_toolBarPlay->create(container);
	m_toolBarPlay->addImage(ui::Bitmap::load(c_ResourcePlayback, sizeof(c_ResourcePlayback), L"png"), 6);
	m_toolBarPlay->addImage(ui::Bitmap::load(c_ResourceAspect, sizeof(c_ResourceAspect), L"png"), 2);
	m_toolBarPlay->addItem(gc_new< ui::custom::ToolBarButton >(L"Rewind", ui::Command(L"Flash.Editor.Rewind"), 0));
	m_toolBarPlay->addItem(gc_new< ui::custom::ToolBarButton >(L"Play", ui::Command(L"Flash.Editor.Play"), 1));
	m_toolBarPlay->addItem(gc_new< ui::custom::ToolBarButton >(L"Stop", ui::Command(L"Flash.Editor.Stop"), 2));
	m_toolBarPlay->addItem(gc_new< ui::custom::ToolBarButton >(L"Forward", ui::Command(L"Flash.Editor.Forward"), 3));
	m_toolBarPlay->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBarPlay->addItem(gc_new< ui::custom::ToolBarButton >(L"Layout Center", ui::Command(L"Flash.Editor.LayoutCenter"), 6));
	m_toolBarPlay->addItem(gc_new< ui::custom::ToolBarButton >(L"Layout Aspect", ui::Command(L"Flash.Editor.LayoutAspect"), 7));
	m_toolBarPlay->addClickEventHandler(ui::createMethodHandler(this, &FlashEditorPage::eventToolClick));

	m_previewContainer = gc_new< ui::Container >();
	m_previewContainer->create(container, ui::WsNone, gc_new< ui::custom::CenterLayout >());

	m_previewControl = gc_new< FlashPreviewControl >();
	m_previewControl->create(m_previewContainer, ui::WsNone, renderSystem);

	return true;
}

void FlashEditorPage::destroy()
{
	m_previewControl->destroy();
	m_resourceCache->flush();
}

void FlashEditorPage::activate()
{
	resource::ResourceManager::getInstance().setCache(m_resourceCache);
	resource::ResourceManager::getInstance().addLoader(m_resourceLoader);
}

void FlashEditorPage::deactivate()
{
	resource::ResourceManager::getInstance().removeLoader(m_resourceLoader);
	resource::ResourceManager::getInstance().setCache(0);
}

bool FlashEditorPage::setDataObject(db::Instance* instance, Object* data)
{
	FlashMovieAsset* asset = checked_type_cast< FlashMovieAsset* >(data);

	Ref< Stream > stream = FileSystem::getInstance().open(asset->getFileName(), File::FmRead);
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

	Ref< MemoryStream > memoryStream = gc_new< MemoryStream >(&assetBlob[0], int(assetSize), true, false);
	Ref< SwfReader > swf = gc_new< SwfReader >(memoryStream);

	m_movie = flash::FlashMovieFactory().createMovie(swf);
	if (!m_movie)
		return false;

	m_previewControl->setMovie(m_movie);
	m_previewContainer->update();

	return true;
}

Object* FlashEditorPage::getDataObject()
{
	return 0;
}

void FlashEditorPage::propertiesChanged()
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
	else if (command == L"Flash.Editor.LayoutCenter")
	{
		m_previewContainer->setLayout(gc_new< ui::custom::CenterLayout >());
		m_previewContainer->update();
	}
	else if (command == L"Flash.Editor.LayoutAspect")
	{
		m_previewContainer->setLayout(gc_new< ui::custom::AspectLayout >());
		m_previewContainer->update();
	}
	else
		result = false;

	return result;
}

void FlashEditorPage::handleDatabaseEvent(const Guid& eventId)
{
	if (m_resourceCache)
		m_resourceCache->flush(eventId);
}

void FlashEditorPage::eventToolClick(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	handleCommand(command);
}

	}
}
