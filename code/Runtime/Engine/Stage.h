/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Class/Any.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

}

namespace traktor::render
{

class RenderGraph;
class ScreenRenderer;
class Shader;

}

namespace traktor::runtime
{

class IEnvironment;
class IStateManager;
class Layer;
class StageLoader;
class UpdateControl;
class UpdateInfo;

/*! Stage class.
 * \ingroup Runtime
 *
 * Stages describe a current state of the application;
 * it contains a ordered list of layers which describe the current
 * high-level setup of the application.
 */
class T_DLLCLASS Stage : public Object
{
	T_RTTI_CLASS;

public:
	Stage(
		const std::wstring& name,
		IEnvironment* environment,
		const resource::Proxy< IRuntimeClass >& clazz,
		const resource::Proxy< render::Shader >& shaderFade,
		bool fadeOutUpdate,
		float fadeRate,
		const SmallMap< std::wstring, Guid >& transitions,
		const Object* params
	);

	virtual ~Stage();

	void destroy();

	/*! Add layer to stage.
	 *
	 * \param layer Layer to add.
	 */
	void addLayer(Layer* layer);

	/*! Remove layer from stage.
	 *
	 * \param layer Layer to remove.
	 */
	void removeLayer(Layer* layer);

	/*! Remove all layers from stage. */
	void removeAllLayers();

	/*! Get layer by name.
	 *
	 * \param name Name of layer ot find.
	 * \return Layer object if found; null if not found.
	 */
	Layer* getLayer(const std::wstring& name) const;

	/*! Signal that the application should terminate. */
	void terminate();

	/*! Invoke a function in stage's script.
	 *
	 * \param fn Script function name.
	 * \param argc Number of arguments.
	 * \param argv Arguments.
	 * \return Return value from script function.
	 */
	Any invokeScript(const std::string& fn, uint32_t argc, const Any* argv);

	/*! Check if transition is defined. */
	bool haveTransition(const std::wstring& name) const;

	/*! Load next stage.
	 *
	 * Next stage is determined by this stage's "transitions"; each
	 * transition has a name and is used to link to other stages.
	 *
	 * \param name Name of transition.
	 * \param params User-defined parameter object which is passed to next stage's "initialize" call.
	 * \return Next stage.
	 */
	Ref< Stage > loadStage(const std::wstring& name, const Object* params);

	/*! Asynchronously load next stage.
	 *
	 * Next stage is determined by this stage's "transitions"; each
	 * transition has a name and is used to link to other stages.
	 *
	 * As this call is asynchronous it will return a "stage loader"
	 * object which can be polled until next stage has been successfully loaded.
	 *
	 * \param name Name of transition.
	 * \param params User-defined parameter object which is passed to next stage's "initialize" call.
	 * \return Stage loader object.
	 */
	Ref< StageLoader > loadStageAsync(const std::wstring& name, const Object* params);

	/*! Leave this stage and enter another stage.
	 *
	 * \param stage Next stage.
	 */
	bool gotoStage(Stage* stage);

	/*! Update this stage.
	 *
	 * \param stateManager Engine state manager.
	 * \param info Engine update information.
	 * \return True if this update succeeded.
	 */
	bool update(IStateManager* stateManager, const UpdateInfo& info);

	/*! Update, post physics, this stage.
	 *
	 * \param stateManager Engine state manager.
	 * \param info Engine update information.
	 * \return True if this update succeeded.
	 */
	bool postUpdate(IStateManager* stateManager, const UpdateInfo& info);

	bool setup(const UpdateInfo& info, render::RenderGraph& renderGraph);

	void transition();

	void preReconfigured();

	void postReconfigured();

	void suspend();

	void resume();

	void hotReload();

	const std::wstring& getName() const { return m_name; }

	IEnvironment* getEnvironment() { return m_environment; }

	const RefArray< Layer >& getLayers() const { return m_layers; }

	const Object* getParams() const { return m_params; }

private:
	std::wstring m_name;
	Ref< IEnvironment > m_environment;
	resource::Proxy< IRuntimeClass > m_class;
	Ref< ITypedObject > m_object;
	Ref< render::ScreenRenderer > m_screenRenderer;
	resource::Proxy< render::Shader > m_shaderFade;
	bool m_fadeOutUpdate;
	float m_fadeRate;
	RefArray< Layer > m_layers;
	SmallMap< std::wstring, Guid > m_transitions;
	Ref< const Object > m_params;
	Ref< Stage > m_pendingStage;
	Ref< Stage > m_transitionStage;
	bool m_initialized;
	bool m_running;
	float m_fade;

	bool validateScriptContext();
};

}
