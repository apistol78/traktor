#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/SwfReader.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionObject.h"
#include "Flash/Action/Classes/Array.h"
#include "Flash/Editor/FlashEditorPage.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Flash/Editor/FlashPreviewControl.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Sound/SoundSystem.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"
#include "Ui/TreeView.h"
#include "Ui/TreeViewItem.h"
#include "Ui/Custom/AspectLayout.h"
#include "Ui/Custom/CenterLayout.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
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

	m_soundSystem = m_editor->getStoreObject< sound::SoundSystem >(L"SoundSystem");

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
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(L"Rewind", 0, ui::Command(L"Flash.Editor.Rewind")));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(L"Play", 1, ui::Command(L"Flash.Editor.Play")));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(L"Stop", 2, ui::Command(L"Flash.Editor.Stop")));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(L"Forward", 3, ui::Command(L"Flash.Editor.Forward")));
	m_toolBarPlay->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &FlashEditorPage::eventToolClick);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(container, true, 300);

	Ref< ui::custom::Splitter > splitterV = new ui::custom::Splitter();
	splitterV->create(splitter, false, -100);

	m_treeMovie = new ui::TreeView();
	m_treeMovie->create(splitterV, ui::TreeView::WsDefault & ~ui::WsClientBorder);
	m_treeMovie->addEventHandler< ui::SelectionChangeEvent >(this, &FlashEditorPage::eventTreeMovieSelect);

	m_profileMovie = new ui::custom::ProfileControl();
	m_profileMovie->create(splitterV, 2, 10, 0, 10000, ui::WsDoubleBuffer, this);

	m_previewControl = new FlashPreviewControl();
	m_previewControl->create(splitter, ui::WsNone, database, m_resourceManager, renderSystem, m_soundSystem);
	m_previewControl->setMovie(m_movie);
	m_previewControl->update();

	return true;
}

void FlashEditorPage::destroy()
{
	safeDestroy(m_previewControl);
	m_soundSystem = 0;
	log::info << FlashCharacterInstance::getInstanceCount() << L" leaked character(s)" << Endl;
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
	{
		m_previewControl->rewind();
		updateTreeMovie();
	}
	else if (command == L"Flash.Editor.Play")
	{
		m_previewControl->play();
		updateTreeMovie();
	}
	else if (command == L"Flash.Editor.Stop")
	{
		m_previewControl->stop();
		updateTreeMovie();
	}
	else if (command == L"Flash.Editor.Forward")
	{
		m_previewControl->forward();
		updateTreeMovie();
	}
	else
		result = false;

	return result;
}

void FlashEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->reload(eventId, false);
}

void FlashEditorPage::getProfileValues(uint32_t* outValues) const
{
	outValues[0] = Collectable::getInstanceCount();
	outValues[1] = FlashCharacterInstance::getInstanceCount();
}

void FlashEditorPage::updateTreeObject(ui::TreeViewItem* parentItem, const ActionObject* asObject, std::set< const ActionObject* >& objectStack, std::map< const void*, uint32_t >& pointerHash, uint32_t& nextPointerHash)
{
	if (!asObject)
		return;

	const ActionObject::member_map_t& asMembers = asObject->getLocalMembers();
	for (ActionObject::member_map_t::const_iterator i = asMembers.begin(); i != asMembers.end(); ++i)
	{
		std::string memberName = asObject->getContext()->getString(i->first);

		StringOutputStream ss;
		ss << mbstows(memberName) << L" = \"" << i->second.getWideString() << L"\"";

		if (i->second.isObject())
		{
			std::map< const void*, uint32_t >::const_iterator j = pointerHash.find(i->second.getObject());
			if (j != pointerHash.end())
				ss << L" @" << j->second;
			else
			{
				uint32_t hash = nextPointerHash++;
				pointerHash.insert(std::make_pair(i->second.getObject(), hash));
				ss << L" @" << hash;
			}
		}

		Ref< ui::TreeViewItem > memberItem = m_treeMovie->createItem(
			parentItem,
			ss.str(),
			0,
			0
		);

		if (i->second.isObject())
		{
			if (objectStack.find(i->second.getObject()) == objectStack.end())
			{
				objectStack.insert(i->second.getObject());
				updateTreeObject(memberItem, i->second.getObject(), objectStack, pointerHash, nextPointerHash);
			}
		}
	}

	const Array* asArray = asObject->getRelay< Array >();
	if (asArray)
	{
		const AlignedVector< ActionValue >& asValues = asArray->getValues();
		for (uint32_t i = 0; i < asValues.size(); ++i)
		{
			StringOutputStream ss;
			ss << L"[" << i << L"] = \"" << asValues[i].getWideString() << L"\"";

			if (asValues[i].isObject())
			{
				std::map< const void*, uint32_t >::const_iterator j = pointerHash.find(asValues[i].getObject());
				if (j != pointerHash.end())
					ss << L" @" << j->second;
				else
				{
					uint32_t hash = nextPointerHash++;
					pointerHash.insert(std::make_pair(asValues[i].getObject(), hash));
					ss << L" @" << hash;
				}
			}

			Ref< ui::TreeViewItem > memberItem = m_treeMovie->createItem(
				parentItem,
				ss.str(),
				0,
				0
			);

			if (asValues[i].isObject())
			{
				if (objectStack.find(asValues[i].getObject()) == objectStack.end())
				{
					objectStack.insert(asValues[i].getObject());
					updateTreeObject(memberItem, asValues[i].getObject(), objectStack, pointerHash, nextPointerHash);
				}
			}
		}
	}
}

void FlashEditorPage::updateTreeCharacter(ui::TreeViewItem* parentItem, FlashCharacterInstance* characterInstance, std::map< const void*, uint32_t >& pointerHash, uint32_t& nextPointerHash)
{
	StringOutputStream ss;
	ss << type_name(characterInstance);
	
	if (!characterInstance->getName().empty())
		ss << std::wstring(L" \"") << mbstows(characterInstance->getName()) << std::wstring(L"\"");

	const ActionObject* asObject = characterInstance->getAsObject();
	if (asObject)
	{
		std::map< const void*, uint32_t >::const_iterator j = pointerHash.find(asObject);
		if (j != pointerHash.end())
			ss << L" @" << j->second;
		else
		{
			uint32_t hash = nextPointerHash++;
			pointerHash.insert(std::make_pair(asObject, hash));
			ss << L" @" << hash;
		}
	}

	Ref< ui::TreeViewItem > characterItem = m_treeMovie->createItem(parentItem, ss.str(), 0, 0);
	T_ASSERT (characterItem);

	characterItem->setData(L"CHARACTER", characterInstance);

	if (FlashSpriteInstance* spriteInstance = dynamic_type_cast< FlashSpriteInstance* >(characterInstance))
	{
		const FlashSprite* sprite = spriteInstance->getSprite();
		if (sprite)
		{
			uint32_t frameCount = sprite->getFrameCount();
			m_treeMovie->createItem(characterItem, toString(frameCount) + L" frame(s)");

			Ref< ui::TreeViewItem > labelsItem = m_treeMovie->createItem(characterItem, L"Label(s)", 0, 0);
			for (uint32_t i = 0; i < frameCount; ++i)
			{
				const FlashFrame* frame = sprite->getFrame(i);
				if (frame)
				{
					const std::string& label = frame->getLabel();
					if (!label.empty())
						m_treeMovie->createItem(labelsItem, toString(i) + L". \"" + mbstows(label) + L"\"");
				}
			}
		}

		Ref< ui::TreeViewItem > layersItem = m_treeMovie->createItem(characterItem, L"Layer(s)", 0, 0);

		const FlashDisplayList::layer_map_t& layers = spriteInstance->getDisplayList().getLayers();
		for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			Ref< ui::TreeViewItem > layerItem = m_treeMovie->createItem(layersItem, toString(i->first), 0, 0);
			T_ASSERT (layerItem);

			if (i->second.instance)
				updateTreeCharacter(layerItem, i->second.instance, pointerHash, nextPointerHash);
		}
	}

	if (asObject)
	{
		Ref< ui::TreeViewItem > membersItem = m_treeMovie->createItem(characterItem, L"Member(s)", 0, 0);
		std::set< const ActionObject* > objectStack;
		updateTreeObject(membersItem, asObject, objectStack, pointerHash, nextPointerHash);
	}
}

void FlashEditorPage::updateTreeMovie()
{
	m_treeMovie->removeAllItems();
	if (!m_previewControl->playing())
	{
		FlashMoviePlayer* moviePlayer = m_previewControl->getMoviePlayer();
		if (moviePlayer)
		{
			FlashSpriteInstance* movieInstance = moviePlayer->getMovieInstance();
			if (movieInstance)
			{
				std::map< const void*, uint32_t > pointerHash;
				uint32_t nextPointerHash = 0;

				Ref< ui::TreeViewItem > memberItemRoot = m_treeMovie->createItem(0, L"_root", 0, 0);
				updateTreeCharacter(memberItemRoot, movieInstance, pointerHash, nextPointerHash);

				ActionContext* actionContext = movieInstance->getContext();
				T_ASSERT (actionContext);

				ActionObject* global = actionContext->getGlobal();
				T_ASSERT (global);

				Ref< ui::TreeViewItem > memberItemGlobal = m_treeMovie->createItem(0, L"_global", 0, 0);
				std::set< const ActionObject* > objectStack;
				updateTreeObject(memberItemGlobal, global, objectStack, pointerHash, nextPointerHash);

				log::info << L"Last object index @" << nextPointerHash << Endl;
			}
		}
	}
	else if (m_selectedCharacterInstance)
	{
		m_selectedCharacterInstance->setColorTransform(m_selectedCharacterInstanceCxForm);
		m_selectedCharacterInstance = 0;
		m_previewControl->update();
	}
}

void FlashEditorPage::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void FlashEditorPage::eventTreeMovieSelect(ui::SelectionChangeEvent* event)
{
	ui::TreeViewItem* selectedItem = m_treeMovie->getSelectedItem();

	if (m_selectedCharacterInstance)
	{
		m_selectedCharacterInstance->setColorTransform(m_selectedCharacterInstanceCxForm);
		m_selectedCharacterInstance = 0;
	}

	if (selectedItem)
	{
		m_selectedCharacterInstance = selectedItem->getData< FlashCharacterInstance >(L"CHARACTER");
		if (m_selectedCharacterInstance)
		{
			m_selectedCharacterInstanceCxForm = m_selectedCharacterInstance->getColorTransform();

			SwfCxTransform cxform;
			cxform.red[0] = 0.5f; cxform.red[1] = 0.0f;
			cxform.green[0] = 0.5f; cxform.green[1] = 0.5f;
			cxform.blue[0] = 0.5f; cxform.blue[1] = 1.0f;
			cxform.alpha[0] = 1.0f; cxform.alpha[1] = 0.0f;

			m_selectedCharacterInstance->setColorTransform(cxform);
		}
	}

	m_previewControl->update();
}

	}
}
