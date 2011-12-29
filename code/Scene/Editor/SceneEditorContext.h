#ifndef traktor_scene_SceneEditorContext_H
#define traktor_scene_SceneEditorContext_H

#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"
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
		GfDescendants = 1 << 0,
		GfSelectedOnly = 1 << 1,
		GfExternals = 1 << 2,
		GfDefault = (GfDescendants)
	};

	enum AxisEnable
	{
		AeX = 1,
		AeY = 2,
		AeZ = 4,
		AeXYZ = AeX | AeY | AeZ
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

	void setPickEnable(bool pickEnable);

	bool getPickEnable() const;

	void setAxisEnable(uint32_t axisEnable);

	uint32_t getAxisEnable() const;

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

	void cloneSelected();

	/*! \name Render entity. */
	//@{

	EntityAdapter* beginRenderEntity(const world::Entity* entity);

	void endRenderEntity();

	//@}

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

	resource::IResourceManager* getResourceManager() const { return m_resourceManager; }

	render::IRenderSystem* getRenderSystem() const { return m_renderSystem; }

	physics::PhysicsManager* getPhysicsManager() const { return m_physicsManager; }

	RefArray< ISceneEditorProfile >& getEditorProfiles() { return m_editorProfiles; }

	RefArray< ISceneEditorPlugin >& getEditorPlugins() { return m_editorPlugins; }

	Ref< ISceneControllerEditor >& getControllerEditor() { return m_controllerEditor; }

	SceneAsset* getSceneAsset() { return m_sceneAsset; }

	Scene* getScene() { return m_scene; }

	EntityAdapter* getRootEntityAdapter() { return m_rootEntityAdapter; }

	EntityAdapter* getFollowEntityAdapter() { return m_followEntityAdapter; }

	//@}

	/*! \name Events. */
	//@{

	enum Events
	{
		EiPreModify = ui::EiUser + 1,
		EiPostModify = ui::EiUser + 2,
		EiPostFrame = ui::EiUser + 3,
		EiPostBuild = ui::EiUser + 4,
		EiSelect = ui::EiUser + 5
	};

	void raisePreModify();

	void raisePostModify();

	void raisePostFrame(ui::Event* event);

	void raisePostBuild();

	void raiseSelect(Object* item);

	void addPreModifyEventHandler(ui::EventHandler* eventHandler);

	void addPostModifyEventHandler(ui::EventHandler* eventHandler);

	void addPostFrameEventHandler(ui::EventHandler* eventHandler);

	void addPostBuildEventHandler(ui::EventHandler* eventHandler);

	void addSelectEventHandler(ui::EventHandler* eventHandler);

	//@}

private:
	editor::IEditor* m_editor;
	Ref< editor::IDocument > m_document;
	Ref< db::Database > m_resourceDb;
	Ref< db::Database > m_sourceDb;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::ITexture > m_debugTexture[4];
	Ref< physics::PhysicsManager > m_physicsManager;
	RefArray< ISceneEditorProfile > m_editorProfiles;
	RefArray< ISceneEditorPlugin > m_editorPlugins;
	Ref< ISceneControllerEditor > m_controllerEditor;
	Ref< IModifier > m_modifier;
	bool m_pickEnable;
	uint32_t m_axisEnable;
	SnapMode m_snapMode;
	float m_snapSpacing;
	bool m_physicsEnable;
	bool m_playing;
	float m_timeScale;
	float m_time;
	Ref< Camera > m_cameras[4];
	Ref< SceneAsset > m_sceneAsset;
	Ref< Scene > m_scene;
	Ref< EntityAdapter > m_rootEntityAdapter;
	Ref< EntityAdapter > m_followEntityAdapter;
	RefArray< EntityAdapter > m_renderEntityStack;
};

	}
}

#endif	// traktor_scene_SceneEditorContext_H
