/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Runtime/IState.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderContext;
class RenderGraph;

	}

	namespace runtime
	{

class IEnvironment;
class Stage;

/*! \brief
 * \ingroup Runtime
 */
class T_DLLCLASS StageState : public IState
{
	T_RTTI_CLASS;

public:
	StageState(IEnvironment* environment, Stage* stage);

	virtual ~StageState();

	virtual void enter() override final;

	virtual void leave() override final;

	virtual UpdateResult update(IStateManager* stateManager, const UpdateInfo& info) override final;

	virtual BuildResult build(uint32_t frame, const UpdateInfo& info) override final;

	virtual bool render(uint32_t frame, const UpdateInfo& info) override final;

	virtual bool take(const Object* event) override final;

private:
	struct Frame
	{
		Ref< render::RenderContext > renderContext;
	};

	Ref< IEnvironment > m_environment;
	Ref< Stage > m_stage;
	AlignedVector< Frame > m_frames;
	Ref< render::RenderGraph > m_renderGraph;
};

	}
}

