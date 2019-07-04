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

class Thread;

    namespace editor
    {

class IEditor;

    }

    namespace shape
    {

class TracerTask;

class T_DLLCLASS TracerProcessor : public Object
{
    T_RTTI_CLASS;

public:
    TracerProcessor(editor::IEditor* editor);

    virtual ~TracerProcessor();

	void enqueue(const TracerTask* task);

	void cancelAll();

	void waitUntilIdle();

private:
    editor::IEditor* m_editor;
	const TypeInfo* m_rayTracerType;
    Thread* m_thread;
    Semaphore m_lock;
    Event m_event;
    RefArray< const TracerTask > m_tasks;

    void processorThread();

    bool process(const TracerTask* task) const;
};

    }
}
