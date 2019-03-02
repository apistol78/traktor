#include "Flash/Collectable.h"
#include "Flash/GC.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Collectable", Collectable, Object)

int32_t Collectable::ms_instanceCount = 0;

Collectable::Collectable()
:	m_prev(0)
,	m_next(0)
,	m_weakRefDisposes(0)
,	m_traceRefCount(0)
,	m_traceColor(TcBlack)
,	m_traceBuffered(false)
{
	Atomic::increment(ms_instanceCount);
}

Collectable::~Collectable()
{
	T_ASSERT (m_next == 0);
	T_ASSERT (m_prev == 0);
	Atomic::decrement(ms_instanceCount);
	delete m_weakRefDisposes;
}

void Collectable::addRef(void* owner) const
{
	m_traceColor = TcBlack;
	Object::addRef(owner);
}

void Collectable::release(void* owner) const
{
	int32_t referenceCount = getReferenceCount();
	if (referenceCount <= 1)
	{
		m_traceColor = TcBlack;
		if (m_traceBuffered)
		{
			m_traceBuffered = false;
			GC::getInstance().removeCandidate(const_cast< Collectable* >(this));
		}

		if (m_weakRefDisposes)
		{
			for (SmallSet< IWeakRefDispose* >::iterator i = m_weakRefDisposes->begin(); i != m_weakRefDisposes->end(); ++i)
				(*i)->disposeReference(const_cast< Collectable* >(this));
		}
	}
	else
	{
		m_traceColor = TcPurple;
		if (!m_traceBuffered)
		{
			m_traceBuffered = true;
			GC::getInstance().addCandidate(const_cast< Collectable* >(this));
		}
	}

	Object::release(owner);
}

void Collectable::addWeakRef(IWeakRefDispose* weakRefDispose)
{
	if (!m_weakRefDisposes)
		m_weakRefDisposes = new SmallSet< IWeakRefDispose* >();
	m_weakRefDisposes->insert(weakRefDispose);
}

void Collectable::releaseWeakRef(IWeakRefDispose* weakRefDispose)
{
	T_ASSERT (m_weakRefDisposes);
	m_weakRefDisposes->erase(weakRefDispose);
}

int32_t Collectable::getInstanceCount()
{
	return ms_instanceCount;
}

void Collectable::traceMarkGray()
{
	if (m_traceColor != TcGray)
	{
		m_traceColor = TcGray;
		m_traceRefCount = getReferenceCount();
		trace(&visitorMarkGray);
	}
}

void Collectable::traceScan()
{
	if (m_traceColor == TcGray)
	{
		if (m_traceRefCount > 0)
			traceScanBlack();
		else
		{
			m_traceColor = TcWhite;
			trace(&visitorScan);
		}
	}
}

void Collectable::traceScanBlack()
{
	m_traceColor = TcBlack;
	trace(&visitorScanBlack);
}

void Collectable::traceCollectWhite()
{
	if (m_traceColor == TcWhite)
	{
		T_ASSERT (m_traceRefCount == 0);
		T_EXCEPTION_GUARD_BEGIN;

		Collectable::addRef(0);
		dereference();
		Collectable::release(0);

		T_EXCEPTION_GUARD_END;
	}
}

void Collectable::visitorMarkGray(Collectable* memberObject)
{
	if (memberObject)
	{
		memberObject->traceMarkGray();
		memberObject->m_traceRefCount--;
	}
}

void Collectable::visitorScan(Collectable* memberObject)
{
	if (memberObject)
		memberObject->traceScan();
}

void Collectable::visitorScanBlack(Collectable* memberObject)
{
	if (memberObject)
	{
		memberObject->m_traceRefCount++;
		if (memberObject->m_traceColor != TcBlack)
			memberObject->traceScanBlack();
	}
}

	}
}
