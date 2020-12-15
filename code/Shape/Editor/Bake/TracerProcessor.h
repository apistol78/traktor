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
class PropertyGroup;
class Thread;

    namespace shape
    {

class TracerTask;

class T_DLLCLASS TracerProcessor : public Object
{
    T_RTTI_CLASS;

public:
	struct Status
	{
		bool active;
		uint32_t current;
		uint32_t total;
		std::wstring description;

        Status()
        :   active(false)
        ,   current(0)
        ,   total(0)
        {
        }
	};

    TracerProcessor(const TypeInfo* rayTracerType, const std::wstring& compressionMethod, bool parallel);

    virtual ~TracerProcessor();

	void enqueue(const TracerTask* task);

    void cancel(const Guid& sceneId);

	void cancelAll();

	void waitUntilIdle();

	Status getStatus() const;

private:
	const TypeInfo* m_rayTracerType;
    std::wstring m_compressionMethod;
    bool m_parallel;
    Thread* m_thread;
    Semaphore m_lock;
    Event m_event;
    RefArray< const TracerTask > m_tasks;
	Ref< const TracerTask > m_activeTask;
	Status m_status;
    bool m_cancelled;

    void processorThread();

    bool process(const TracerTask* task);
};

    }
}
