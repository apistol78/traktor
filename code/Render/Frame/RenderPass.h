/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/StaticVector.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class RenderContext;
class RenderGraph;

/*! Render pass definition.
 * \ingroup Render
 */
class T_DLLCLASS RenderPass : public RefCountImpl< ITypedObject >
{
	T_RTTI_CLASS;

public:
	typedef std::function< void(const RenderGraph&, RenderContext*) > fn_build_t;

	struct Input
	{
		handle_t resourceId = 0;	//!< Resource name.
	};

	struct Output
	{
		handle_t resourceId = ~0;	//!< Resource name.
		Clear clear;				//!< Target clear value and mask (only applicable to render target resources).
		uint32_t load = 0;
		uint32_t store = 0;
	};

	explicit RenderPass(const std::wstring& name = L"Unnamed");

	void setName(const std::wstring& name);

	const std::wstring& getName() const { return m_name; }

	//! \{

	void addInput(handle_t resourceId);

	bool requireInput(handle_t resourceId) const;

	const StaticVector< Input, 16 >& getInputs() const { return m_inputs; }

	//! \}

	//! \{

	void setOutput(handle_t resourceId);

	void setOutput(handle_t resourceId, uint32_t load, uint32_t store);

	void setOutput(handle_t resourceId, const Clear& clear, uint32_t load, uint32_t store);

	bool haveOutput() const { return m_output.resourceId != ~0; }

	const Output& getOutput() const { return m_output; }

	//! \}

	//! \{

	void addBuild(const fn_build_t& build);

	const AlignedVector< fn_build_t >& getBuilds() const { return m_builds; }

	//! \}

	void* operator new (size_t size);

	void operator delete (void* ptr);

protected:
	std::wstring m_name;
	StaticVector< Input, 16 > m_inputs;
	Output m_output;
    AlignedVector< fn_build_t > m_builds;
};

}
