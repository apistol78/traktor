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
#include "Core/Thread/Event.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Guid;
class JobQueue;
class PropertyGroup;
class Thread;

}

namespace traktor::shape
{

class TracerTask;

class T_DLLCLASS TracerProcessor : public Object
{
	T_RTTI_CLASS;

public:
	struct Status
	{
		bool active = false;
		uint32_t current = 0;
		uint32_t total = 0;
		Guid scene;
		std::wstring description;
		double lastDuration = 0.0;
	};

	explicit TracerProcessor(const TypeInfo* rayTracerType, const std::wstring& compressionMethod, bool editor);

	virtual ~TracerProcessor();

	void enqueue(const TracerTask* task);

	void cancel(const Guid& sceneId);

	void cancelAll();

	void waitUntilIdle();

	Status getStatus() const;

private:
	const TypeInfo* m_rayTracerType = nullptr;
	std::wstring m_compressionMethod;
	bool m_editor = false;
	Thread* m_thread = nullptr;
	Ref< JobQueue > m_queue;
	Semaphore m_lock;
	Event m_event;
	RefArray< const TracerTask > m_tasks;
	Ref< const TracerTask > m_activeTask;
	Status m_status;
	bool m_cancelled = false;

	void processorThread();

	bool process(const TracerTask* task);
};

}
