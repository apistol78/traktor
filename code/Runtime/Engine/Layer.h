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
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class RenderGraph;

}

namespace traktor::runtime
{

class Stage;
class UpdateControl;
class UpdateInfo;

/*! Abstract stage layer.
 * \ingroup Runtime
 */
class T_DLLCLASS Layer : public Object
{
	T_RTTI_CLASS;

public:
	Layer(
		Stage* stage,
		const std::wstring& name,
		bool permitTransition
	);

	virtual ~Layer();

	virtual void destroy();

	/*! Called during transition between stages.
	 *
	 * Transition is called for layers which have the
	 * same type and matching names.
	 *
	 * \param fromLayer Previous stage's layer.
	 */
	virtual void transition(Layer* fromLayer) = 0;

	/*! Prepare layer for update.
	 *
	 * \param info Engine update information.
	 */
	virtual void preUpdate(const UpdateInfo& info) = 0;

	/*! Update layer logic.
	 *
	 * info Engine update information.
	 */
	virtual void update(const UpdateInfo& info) = 0;

	/*! Prepare layer for setup.
	 *
	 * \param info Engine update information.
	 */
	virtual void preSetup(const UpdateInfo& info) = 0;

	/*! Setup render graph passes.
	 *
	 * \param info Engine update information.
	 * \param renderGraph Render graph.
	 */
	virtual void setup(const UpdateInfo& info, render::RenderGraph& renderGraph) = 0;

	/*! Called before application is to be reconfigured. */
	virtual void preReconfigured() = 0;

	/*! Called after configuration has been applied. */
	virtual void postReconfigured() = 0;

	/*! Called when application is to be suspended. */
	virtual void suspend() = 0;

	/*! Called after application has resumed executing. */
	virtual void resume() = 0;

	/*! Get owner stage object.
	 *
	 * \return Stage object.
	 */
	Stage* getStage() const { return m_stage; }

	/*! Get name of this layer.
	 *
	 * \return This layer's name.
	 */
	const std::wstring& getName() const { return m_name; }

	/*! Return flag if transition of this layer is permitted.
	 *
	 * \return True if transition is permitted.
	 */
	bool isTransitionPermitted() const { return m_permitTransition; }

private:
	Stage* m_stage;
	std::wstring m_name;
	bool m_permitTransition;
};

}
