#ifndef traktor_scene_SceneEditorContext_H
#define traktor_scene_SceneEditorContext_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Frustum.h"
#include "Ui/EventSubject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;
class Instance;

	}

	namespace editor
	{

class IDocument;
class IEditor;

	}

	namespace physics
	{

class PhysicsManager;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class ITexture;
class PrimitiveRenderer;

	}

	namespace world
	{

class Entity;
class EntityData;
class IEntityEventManager;

	}

	namespace scene
	{

class Camera;
class EntityAdapter;
class IModifier;
class ISceneControllerEditor;
class ISceneEditorPlugin;
class ISceneEditorProfile;
class Scene;
class SceneAsset;

/*! \brief Scene editor context.
 *
 * Scene entities, editor profiles and entity editors are kept
 * by the scene editor context.
 */
class T_DLLCLASS SceneEditorContext : public ui::EventSubject
{
	T_RTTI_CLASS;

public:
	enum GetEntitiesFlags
	{
		GfNoDescendants = 0,
		GfDescendants = 1 << 0,
		GfSelectedOnly = 1 << 1,
		GfNoSelected = 1 << 2,
		GfExternalOnly = 1 << 5,
		GfNoExternal = 1 << 6,
		GfExternalChildOnly = 1 << 7,
		GfNoExternalChild = 1 << 8,
		GfDefault = (GfDescendants)
	};

	enum SnapMode
	{
		SmNone,
		SmGrid,
		SmNeighbour
	};

	SceneEditorContext(
		editor::IEditor* editor,
		editor::IDocument* document,
		db::Database* resourceDb,
		db::Database* sourceDb,
		world::IEntityEventManager* eventManager,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		physics::PhysicsManager* physicsManager
	);

	virtual ~SceneEditorContext();

	void destroy();

	void addEditorProfile(ISceneEditorProfile* editorProfile);

	void addEditorPlugin(ISceneEditorPlugin* editorPlugin);

	void setControllerEditor(ISceneControllerEditor* controllerEditor);

	/*! \name State management. */
	//@{

	void setModifier(IModifier* modifier);

	IModifier* getModifier() const;

	void setGuideSize(float guideSize);

	float getGuideSize() const;

	void setPickEnable(bool pickEnable);

	bool getPickEnable() const;

	void setSnapMode(SnapMode snapMode);

	SnapMode getSnapMode() const;

	void setSnapSpacing(float snapSpacing);

	float getSnapSpacing() const;

	void setPhysicsEnable(bool physicsEnable);

	bool getPhysicsEnable() const;

	//@}

	/*! \name Camera control. */
	//@{

	/*! \brief Get camera by index.
	 *
	 * \param index Camera index, 0 to 3.
	 * \return Camera.
	 */
	Camera* getCamera(int index) const;

	void setFollowEntityAdapter(EntityAdapter* followEntityAdapter);

	void setLookAtEntityAdapter(EntityAdapter* lookAtEntityAdapter);

	void moveToEntityAdapter(EntityAdapter* entityAdapter);

	//@}

	/*! \name Time control. */
	//@{

	void setPlaying(bool playing);

	bool isPlaying() const;

	void setTimeScale(float timeScale);

	float getTimeScale() const;

	void setTime(float time);

	float getTime() const;

	//@}

	/*! \name Entity editors. */
	//@{

	void drawGuide(render::PrimitiveRenderer* primitiveRenderer, EntityAdapter* entityAdapter);

	//@}

	void setSceneAsset(SceneAsset* sceneAsset);

	void buildEntities();

	void selectEntity(EntityAdapter* entityAdapter, bool select = true);

	void selectAllEntities(bool select = true);

	/*! \brief Get entities. */
	uint32_t getEntities(RefArray< EntityAdapter >& outEntityAdapters, uint32_t flags = GfDefault) const;
	
	EntityAdapter* findAdapterFromEntity(const world::Entity* entity) const;

	EntityAdapter* queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, bool onlyPickable) const;

	uint32_t queryFrustum(const Frustum& worldFrustum, RefArray< EntityAdapter >& outEntityAdapters, bool onlyPickable) const;

	void cloneSelected();

	/*! \name Debug texture view. */
	//@{

	void setDebugTexture(uint32_t index, render::ITexture* debugTexture);

	render::ITexture* getDebugTexture(uint32_t index);

	//@}

	/*! \name Plugin access. */
	//@{

	ISceneEditorPlugin* getEditorPluginOf(const TypeInfo& pluginType) const;

	template < typename PluginType >
	PluginType* getEditorPluginOf() const
	{
		return dynamic_type_cast< PluginType* >(getEditorPluginOf(type_of< PluginType >()));
	}

	//@}

	/*! \name Accessors. */
	//@{

	editor::IEditor* getEditor() { return m_editor; }

	editor::IDocument* getDocument() { return m_document; }

	db::Database* getResourceDatabase() { return m_resourceDb; }

	db::Database* getSourceDatabase() { return m_sourceDb; }

	world::IEntityEventManager* getEntityEventManager() const { return m_eventManager; }

	resource::IResourceManager* getResourceManager() const { return m_resourceManager; }

	render::IRenderSystem* getRenderSystem() const { return m_renderSystem; }

	physics::PhysicsManager* getPhysicsManager() const { return m_physicsManager; }

	RefArray< ISceneEditorProfile >& getEditorProfiles() { return m_editorProfiles; }

	RefArray< ISceneEditorPlugin >& getEditorPlugins() { return m_editorPlugins; }

	Ref< ISceneControllerEditor >& getControllerEditor() { return m_controllerEditor; }

	SceneAsset* getSceneAsset() { return m_sceneAsset; }

	Scene* getScene() { return m_scene; }

	const RefArray< EntityAdapter > getLayerEntityAdapters() { return m_layerEntityAdapters; }

	EntityAdapter* getFollowEntityAdapter() { return m_followEntityAdapter; }

	EntityAdapter* getLookAtEntityAdapter() { return m_lookAtEntityAdapter; }

	//@}

	/*! \name Events. */
	//@{

	enum Events
	{
		EiPreModify = ui::EiUser + 1,
		EiPostModify = ui::EiUser + 2,
		EiPostFrame = ui::EiUser + 3,
		EiPostBuild = ui::EiUser + 4,
		EiSelect = ui::EiUser + 5,
		EiCameraMoved = ui::EiUser + 6,
		EiModifierChanged = ui::EiUser + 7
	};

	void raisePreModify();

	void raisePostModify();

	void raisePostFrame(ui::Event* event);

	void raisePostBuild();

	void raiseSelect(Object* item);

	void raiseCameraMoved();

	void raiseModifierChanged();

	void addPreModifyEventHandler(ui::EventHandler* eventHandler);

	void addPostModifyEventHandler(ui::EventHandler* eventHandler);

	void addPostFrameEventHandler(ui::EventHandler* eventHandler);

	void addPostBuildEventHandler(ui::EventHandler* eventHandler);

	void addSelectEventHandler(ui::EventHandler* eventHandler);

	void addCameraMovedEventHandler(ui::EventHandler* eventHandler);

	void addModifierChangedEventHandler(ui::EventHandler* eventHandler);

	//@}

private:
	editor::IEditor* m_editor;
	editor::IDocument* m_document;
	Ref< db::Database > m_resourceDb;
	Ref< db::Database > m_sourceDb;
	Ref< world::IEntityEventManager > m_eventManager;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::ITexture > m_debugTexture[4];
	Ref< physics::PhysicsManager > m_physicsManager;
	RefArray< ISceneEditorProfile > m_editorProfiles;
	RefArray< ISceneEditorPlugin > m_editorPlugins;
	Ref< ISceneControllerEditor > m_controllerEditor;
	Ref< IModifier > m_modifier;
	float m_guideSize;
	bool m_pickEnable;
	SnapMode m_snapMode;
	float m_snapSpacing;
	bool m_physicsEnable;
	bool m_playing;
	float m_timeScale;
	float m_time;
	Ref< Camera > m_cameras[4];
	Ref< SceneAsset > m_sceneAsset;
	Ref< Scene > m_scene;
	RefArray< EntityAdapter > m_layerEntityAdapters;
	SmallMap< const world::Entity*, EntityAdapter* > m_entityAdapterMap;
	Ref< EntityAdapter > m_followEntityAdapter;
	Ref< EntityAdapter > m_lookAtEntityAdapter;
};

	}
}

#endif	// traktor_scene_SceneEditorContext_H
