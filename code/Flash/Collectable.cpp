#include "Flash/Collectable.h"
#include "Flash/GC.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Collectable", Collectable, Object)

Collectable::Collectable()
:	m_prev(0)
,	m_next(0)
,	m_traceColor(TcBlack)
,	m_traceBuffered(false)
,	m_traceRefCount(0)
{
}

Collectable::~Collectable()
{
	T_ASSERT (m_next == 0);
	T_ASSERT (m_prev == 0);
}

void Collectable::addRef(void* owner) const
{
	m_traceColor = TcBlack;
	Object::addRef(owner);
}

void Collectable::release(void* owner) const
{
	if (getReferenceCount() <= 1)
	{
		m_traceColor = TcBlack;
		if (m_traceBuffered)
		{
			m_traceBuffered = false;
			GC::getInstance().removeCandidate(const_cast< Collectable* >(this));
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

void Collectable::traceMarkGray()
{
	if (m_traceColor != TcGray)
	{
		m_traceColor = TcGray;
		m_traceRefCount = getReferenceCount();
		trace(MarkGrayVisitor());
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
			trace(ScanVisitor());
		}
	}
}

void Collectable::traceScanBlack()
{
	m_traceColor = TcBlack;
	trace(ScanBlackVisitor());
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

void Collectable::MarkGrayVisitor::operator () (Collectable* memberObject) const
{
	if (memberObject)
	{
		memberObject->traceMarkGray();
		memberObject->m_traceRefCount--;
	}
}

void Collectable::ScanVisitor::operator () (Collectable* memberObject) const
{
	if (memberObject)
		memberObject->traceScan();
}

void Collectable::ScanBlackVisitor::operator () (Collectable* memberObject) const
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
