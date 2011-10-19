#ifndef traktor_flash_Collectable_H
#define traktor_flash_Collectable_H

#include "Core/Object.h"
#include "Core/Containers/IntrusiveList.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class T_DLLCLASS Collectable : public Object
{
	T_RTTI_CLASS;

public:
	struct IVisitor
	{
		virtual void operator () (Collectable* childCollectable) const = 0;
	};

	Collectable();

	virtual ~Collectable();

	virtual void addRef(void* owner) const;

	virtual void release(void* owner) const;

protected:
	virtual void trace(const IVisitor& visitor) const = 0;

	virtual void dereference() = 0;

private:
	friend struct DefaultLink< Collectable >;
	friend class GC;

	enum TraceColor
	{
		TcBlack,
		TcPurple,
		TcGray,
		TcWhite
	};

	struct MarkGrayVisitor : public IVisitor
	{
		virtual void operator () (Collectable* memberObject) const;
	};

	struct ScanVisitor : public IVisitor
	{
		virtual void operator () (Collectable* memberObject) const;
	};

	struct ScanBlackVisitor : public IVisitor
	{
		virtual void operator () (Collectable* memberObject) const;
	};

	Collectable* m_prev;	//!< Intrusive list chain members.
	Collectable* m_next;
	mutable int32_t m_traceColor;
	mutable bool m_traceBuffered;
	mutable int32_t m_traceRefCount;

	void traceMarkGray();

	void traceScan();

	void traceScanBlack();

	void traceCollectWhite();
};

	}
}

#endif	// traktor_flash_Collectable_H
