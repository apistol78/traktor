/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Core/Timer/Timer.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

class BidirectionalObjectTransport;

}

namespace traktor::runtime
{

class T_DLLCLASS TargetPerfSet : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool check(const TargetPerfSet& old) const = 0;
};

class T_DLLCLASS TpsRuntime : public TargetPerfSet
{
	T_RTTI_CLASS;

public:
	float fps = 0.0f;
	float update = 0.0f;
	float build = 0.0f;
	float renderCPU = 0.0f;
	float renderGPU = 0.0f;
	float physics = 0.0f;
	float input = 0.0f;
	float garbageCollect = 0.0f;
	int32_t steps = 0;
	float simulationInterval = 0.0f;
	float renderInterval = 0.0f;
	uint32_t collisions = 0;	//!< Render collisions, when update and render threads collide.

	virtual bool check(const TargetPerfSet& old) const override final;

	virtual void serialize(ISerializer& s) override final;
};

class T_DLLCLASS TpsMemory : public TargetPerfSet
{
	T_RTTI_CLASS;

public:
	uint64_t memInUse = 0;
	uint32_t memInUseScript = 0;
	uint32_t heapObjects = 0;

	virtual bool check(const TargetPerfSet& old) const override final;

	virtual void serialize(ISerializer& s) override final;
};

class T_DLLCLASS TpsRender : public TargetPerfSet
{
	T_RTTI_CLASS;

public:
	render::RenderSystemStatistics renderSystemStats;
	render::RenderViewStatistics renderViewStats;

	virtual bool check(const TargetPerfSet& old) const override final;

	virtual void serialize(ISerializer& s) override final;
};

class T_DLLCLASS TpsResource : public TargetPerfSet
{
	T_RTTI_CLASS;

public:
	uint32_t residentResourcesCount = 0;
	uint32_t exclusiveResourcesCount = 0;

	virtual bool check(const TargetPerfSet& old) const override final;

	virtual void serialize(ISerializer& s) override final;
};

class T_DLLCLASS TpsPhysics : public TargetPerfSet
{
	T_RTTI_CLASS;

public:
	uint32_t bodyCount = 0;
	uint32_t activeBodyCount = 0;
	uint32_t manifoldCount = 0;
	uint32_t queryCount = 0;

	virtual bool check(const TargetPerfSet& old) const override final;

	virtual void serialize(ISerializer& s) override final;
};

class T_DLLCLASS TpsAudio : public TargetPerfSet
{
	T_RTTI_CLASS;

public:
	uint32_t activeSoundChannels = 0;

	virtual bool check(const TargetPerfSet& old) const override final;

	virtual void serialize(ISerializer& s) override final;
};

class T_DLLCLASS TargetPerformance : public Object
{
	T_RTTI_CLASS;

public:
	void publish(net::BidirectionalObjectTransport* transport, const TargetPerfSet& performance);

private:
	struct Snapshot
	{
		double sent;
		Ref< TargetPerfSet > perfSet;
	};

	Timer m_timer;
	SmallMap< const TypeInfo*, Snapshot > m_last;
};

}
