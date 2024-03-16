/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/Engine/Layer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class PrimitiveRenderer;

}

namespace traktor::runtime
{

class IEnvironment;

/*!
 * \ingroup Runtime
 */
class T_DLLCLASS DebugLayer : public Layer
{
	T_RTTI_CLASS;

public:
	explicit DebugLayer(
		Stage* stage,
		IEnvironment* environment
	);

	virtual void destroy() override;

	virtual void transition(Layer* fromLayer) override final;

	virtual void preUpdate(const UpdateInfo& info) override final;

	virtual void update(const UpdateInfo& info) override final;

	virtual void postUpdate(const UpdateInfo& info) override final;

	virtual void preSetup(const UpdateInfo& info) override final;

	virtual void setup(const UpdateInfo& info, render::RenderGraph& renderGraph) override final;

	virtual void preReconfigured() override final;

	virtual void postReconfigured() override final;

	virtual void suspend() override final;

	virtual void resume() override final;

	virtual void hotReload() override final;

	void setColor(const Color4f& color) { m_color = color; }

	const Color4f& getColor() const { return m_color; }

	void setSize(float size) { m_size = size; }

	float getSize() const { return m_size; }

	void addPoint(const Vector4& position);

private:
	struct Point
	{
		Vector4 position;
		Color4f color;
		float size;
	};

	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	int32_t m_count = 0;

	Color4f m_color = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	float m_size = 10.0f;

	AlignedVector< Point > m_points;
};

}
