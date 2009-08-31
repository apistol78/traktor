#ifndef traktor_scene_SceneEditorContext_H
#define traktor_scene_SceneEditorContext_H

#include "Core/Heap/Ref.h"
#include "Core/Math/Vector4.h"
#include "Ui/EventSubject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace db
	{

class Database;

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

	namespace physics
	{

class PhysicsManager;

	}

	namespace world
	{

class EntityInstance;
class EntityData;
class Entity;

	}

	namespace scene
	{

class ISceneEditorProfile;
class IModifier;
class EntityAdapter;
class Scene;
class SceneAsset;

/*! \brief Scene editor context.
 *
 * Scene entities, editor profiles and entity editors are kept
 * by the scene editor context.
 */
class T_DLLCLASS SceneEditorContext : public ui::EventSubject
{
	T_RTTI_CLASS(SceneEditorContext)

public:
	enum GetEntitiesFlags
	{
		GfDescendants = 1 << 0,
		GfSelectedOnly = 1 << 1,
		GfDefault = (GfDescendants)
	};

	enum AxisEnable
	{
		AeX = 1,
		AeY = 2,
		AeZ = 4,
		AeXYZ = AeX | AeY | AeZ
	};

	SceneEditorContext(
		editor::IEditor* editor,
		db::Database* resourceDb,
		db::Database* sourceDb,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		physics::PhysicsManager* physicsManager
	);

	void addEditorProfile(ISceneEditorProfile* editorProfile);
	
	/*! \name State management. */
	//@{

	void setModifier(IModifier* modifier);

	IModifier* getModifier() const;

	void setPickEnable(bool pickEnable);

	bool getPickEnable() const;

	void setAxisEnable(uint32_t axisEnable);

	uint32_t getAxisEnable() const;

	void setGuideEnable(bool guideEnable);

	bool getGuideEnable() const;

	void setSnapEnable(bool snapEnable);

	bool getSnapEnable() const;

	void setPhysicsEnable(bool physicsEnable);

	bool getPhysicsEnable() const;

	void setAddReferenceMode(bool referenceMode);

	bool inAddReferenceMode() const;

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

	EntityAdapter* findAdapterFromInstance(const world::EntityInstance* instance) const;

	EntityAdapter* queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection) const;

	/*! \name Debug texture view. */
	//@{

	void setDebugTexture(render::ITexture* debugTexture);

	render::ITexture* getDebugTexture();

	//@}

	/*! \name Accessors. */
	//@{

	editor::IEditor* getEditor() { return m_editor; }

	db::Database* getResourceDatabase() { return m_resourceDb; }

	db::Database* getSourceDatabase() { return m_sourceDb; }

	resource::IResourceManager* getResourceManager() { return m_resourceManager; }

	render::IRenderSystem* getRenderSystem() { return m_renderSystem; }

	physics::PhysicsManager* getPhysicsManager() { return m_physicsManager; }

	RefArray< ISceneEditorProfile >& getEditorProfiles() { return m_editorProfiles; }

	Ref< SceneAsset >& getSceneAsset() { return m_sceneAsset; }

	Ref< Scene >& getScene() { return m_scene; }

	Ref< EntityAdapter >& getRootEntityAdapter() { return m_rootEntityAdapter; }

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

	void raiseSelect();

	void addPreModifyEventHandler(ui::EventHandler* eventHandler);

	void addPostModifyEventHandler(ui::EventHandler* eventHandler);

	void addPostFrameEventHandler(ui::EventHandler* eventHandler);

	void addPostBuildEventHandler(ui::EventHandler* eventHandler);

	void addSelectEventHandler(ui::EventHandler* eventHandler);

	//@}

private:
	editor::IEditor* m_editor;
	Ref< db::Database > m_resourceDb;
	Ref< db::Database > m_sourceDb;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::ITexture > m_debugTexture;
	Ref< physics::PhysicsManager > m_physicsManager;
	RefArray< ISceneEditorProfile > m_editorProfiles;
	Ref< IModifier > m_modifier;
	bool m_pickEnable;
	uint32_t m_axisEnable;
	bool m_guideEnable;
	bool m_snapEnable;
	bool m_physicsEnable;
	bool m_referenceMode;
	bool m_playing;
	float m_timeScale;
	float m_time;
	Ref< SceneAsset > m_sceneAsset;
	Ref< Scene > m_scene;
	Ref< EntityAdapter > m_rootEntityAdapter;
};

	}
}

#endif	// traktor_scene_SceneEditorContext_H
