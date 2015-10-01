#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Class/OrderedClassRegistrar.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Script/ScriptClassFactory.h"
#include "Script/ScriptModuleFactory.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Sound/SoundFactory.h"
#include "Sound/Player/ISoundPlayer.h"
#include "Spark/CharacterResourceFactory.h"
#include "Spark/FontResourceFactory.h"
#include "Spark/Sprite.h"
#include "Spark/ShapeResourceFactory.h"
#include "Spark/Editor/SparkEditControl.h"
#include "Spark/Editor/SparkEditorPage.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

// Resources
#include "Resources/Playback.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

const struct ViewSize
{
	int32_t width;
	int32_t height;
	const wchar_t* description;
}
c_viewSizes[] =
{
	{ 320, 480, L"320 * 480 (iPhone)" },
	{ 640, 960, L"640 * 960 (iPhone Retina)" },
	{ 640, 1136, L"640 * 1136 (iPhone 5)" },
	{ 750, 1334, L"750 * 1334 (iPhone 6)" },
	{ 1080, 1920, L"1080 * 1920 (iPhone 6+)" },
	{ 768, 1024, L"768 * 1024 (iPad)" },
	{ 1536, 2048, L"1536 * 2048 (iPad Retina)" }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SparkEditorPage", SparkEditorPage, editor::IEditorPage)

SparkEditorPage::SparkEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool SparkEditorPage::create(ui::Container* parent)
{
	Ref< render::IRenderSystem > renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	Ref< sound::ISoundPlayer > soundPlayer = m_editor->getStoreObject< sound::ISoundPlayer >(L"SoundPlayer");

	Ref< db::Database > database = m_editor->getOutputDatabase();

	m_scriptManager = new script::ScriptManagerLua();

	// Register all runtime classes, first collect all classes
	// and then register them in class dependency order.
	OrderedClassRegistrar registrar;
	std::set< const TypeInfo* > runtimeClassFactoryTypes;
	type_of< IRuntimeClassFactory >().findAllOf(runtimeClassFactoryTypes, false);
	for (std::set< const TypeInfo* >::const_iterator i = runtimeClassFactoryTypes.begin(); i != runtimeClassFactoryTypes.end(); ++i)
	{
		Ref< IRuntimeClassFactory > runtimeClassFactory = dynamic_type_cast< IRuntimeClassFactory* >((*i)->createInstance());
		if (runtimeClassFactory)
			runtimeClassFactory->createClasses(&registrar);
	}
	registrar.registerClassesInOrder(m_scriptManager);

	m_resourceManager = new resource::ResourceManager(true);
	m_resourceManager->addFactory(new script::ScriptClassFactory(database, m_scriptManager->createContext()));
	m_resourceManager->addFactory(new script::ScriptModuleFactory(database, m_scriptManager));
	m_resourceManager->addFactory(new render::ShaderFactory(database, renderSystem));
	m_resourceManager->addFactory(new render::TextureFactory(database, renderSystem, 0));
	m_resourceManager->addFactory(new sound::SoundFactory(database));
	m_resourceManager->addFactory(new CharacterResourceFactory(database));
	m_resourceManager->addFactory(new FontResourceFactory(database, renderSystem));
	m_resourceManager->addFactory(new ShapeResourceFactory(database, renderSystem));

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(container, ui::WsNone);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourcePlayback, sizeof(c_ResourcePlayback), L"png"), 6);

	m_toolViewSize = new ui::custom::ToolBarDropDown(ui::Command(L"Spark.Editor.ViewSize"), ui::scaleBySystemDPI(220), i18n::Text(L"SPARK_EDITOR_VIEW_SIZE"));
	for (int32_t i = 0; i < sizeof_array(c_viewSizes); ++i)
		m_toolViewSize->add(c_viewSizes[i].description);
	m_toolViewSize->select(0);

	m_toolViewLandscape = new ui::custom::ToolBarButton(L"Landscape", ui::Command(L"Spark.Editor.ViewSize"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggle);
	m_toolTogglePlay = new ui::custom::ToolBarButton(L"Toggle Play", 1, ui::Command(L"Spark.Editor.TogglePlay"));

	m_toolBar->addItem(m_toolViewSize);
	m_toolBar->addItem(m_toolViewLandscape);
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Rewind", 0, ui::Command(L"Spark.Editor.Rewind")));
	m_toolBar->addItem(m_toolTogglePlay);

	m_toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &SparkEditorPage::eventToolClick);

	m_editControl = new SparkEditControl(m_editor, m_site);
	m_editControl->create(container, ui::WsNone, database, m_resourceManager, renderSystem, soundPlayer);
	m_editControl->setViewSize(c_viewSizes[0].width, c_viewSizes[0].height);
	m_editControl->update();

	Ref< Sprite > sprite = m_document->getObject< Sprite >(0);
	m_editControl->setSprite(sprite);

	m_site->setPropertyObject(sprite);
	return true;
}

void SparkEditorPage::destroy()
{
	safeDestroy(m_editControl);
	safeDestroy(m_resourceManager);
	safeDestroy(m_scriptManager);
}

void SparkEditorPage::activate()
{
}

void SparkEditorPage::deactivate()
{
}

bool SparkEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return m_editControl->dropInstance(instance, position);
}

bool SparkEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.PropertiesChanged")
		m_editControl->refresh();
	else
		return false;

	return true;
}

void SparkEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->reload(eventId, false);
}

void SparkEditorPage::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	if (event->getCommand() == L"Spark.Editor.ViewSize")
	{
		int32_t index = m_toolViewSize->getSelected();
		if (!m_toolViewLandscape->isToggled())
			m_editControl->setViewSize(c_viewSizes[index].width, c_viewSizes[index].height);
		else
			m_editControl->setViewSize(c_viewSizes[index].height, c_viewSizes[index].width);
	}
	else if (event->getCommand() == L"Spark.Editor.TogglePlay")
	{
		if (m_editControl->isPlaying())
		{
			if (m_editControl->stop())
				m_toolTogglePlay->setImage(1);
		}
		else
		{
			if (m_editControl->play())
				m_toolTogglePlay->setImage(2);
		}
		m_toolBar->update();
	}
	else if (event->getCommand() == L"Spark.Editor.Rewind")
	{
		if (m_editControl->rewind())
		{
			m_toolTogglePlay->setImage(1);
			m_toolBar->update();
		}
	}
}

	}
}