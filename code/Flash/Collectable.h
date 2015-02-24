#ifndef traktor_flash_Collectable_H
#define traktor_flash_Collectable_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
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
		virtual ~IVisitor() {}

		virtual void operator () (Collectable* childCollectable) const = 0;
	};

	struct IWeakRefDispose
	{
		virtual ~IWeakRefDispose() {}

		virtual void disposeReference(Collectable* collectable) = 0;
	};

	Collectable();

	virtual ~Collectable();

	virtual void addRef(void* owner) const;

	virtual void release(void* owner) const;

	void addWeakRef(IWeakRefDispose* weakRefDispose);

	void releaseWeakRef(IWeakRefDispose* weakRefDispose);

	/*! \brief Get alive collectible instance count. */
	static int32_t getInstanceCount();

protected:
	typedef void (*visitor_t)(Collectable* memberObject);

	virtual void trace(/*const IVisitor& visitor*/ visitor_t visitor) const = 0;

	virtual void dereference() = 0;

private:
	friend struct DefaultLink< Collectable >;
	friend class GC;

	enum TraceColor
	{
		TcBlack = 0,
		TcPurple = 1,
		TcGray = 2,
		TcWhite = 3
	};

	//struct MarkGrayVisitor : public IVisitor
	//{
	//	virtual void operator () (Collectable* memberObject) const;
	//};

	//struct ScanVisitor : public IVisitor
	//{
	//	virtual void operator () (Collectable* memberObject) const;
	//};

	//struct ScanBlackVisitor : public IVisitor
	//{
	//	virtual void operator () (Collectable* memberObject) const;
	//};

	static int32_t ms_instanceCount;
	Collectable* m_prev;	//!< Intrusive list chain members.
	Collectable* m_next;
	mutable AlignedVector< IWeakRefDispose* > m_weakRefDisposes;
	mutable TraceColor m_traceColor;
	mutable bool m_traceBuffered;
	mutable int32_t m_traceRefCount;

	void traceMarkGray();

	void traceScan();

	void traceScanBlack();

	void traceCollectWhite();

	static void visitorMarkGray(Collectable* memberObject);

	static void visitorScan(Collectable* memberObject);

	static void visitorScanBlack(Collectable* memberObject);
};

	}
}

#endif	// traktor_flash_Collectable_H
