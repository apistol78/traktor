/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Sound/ISoundResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class Graph;

/*! \ingroup Sound */
class T_DLLCLASS GraphResource : public ISoundResource
{
	T_RTTI_CLASS;

public:
	GraphResource() = default;

	explicit GraphResource(
		const std::wstring& category,
		float gain,
		float range,
		const Graph* graph
	);

	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const override final;

	virtual void serialize(ISerializer& s) override final;

	const Graph* getGraph() const { return m_graph; }

private:
	std::wstring m_category;
	float m_gain = 0.0f;
	float m_range = 0.0f;
	Ref< const Graph > m_graph;
};

}
