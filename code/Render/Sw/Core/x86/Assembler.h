/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Assembler_H
#define traktor_render_Assembler_H

#include <vector>
#include <map>
#include "Core/Config.h"

namespace traktor
{
	namespace render
	{

/*! \brief CPU general register abstraction.
 * \ingroup SW
 */
class Register
{
public:
	Register(int32_t index);

	int32_t getIndex() const;

	bool operator == (const Register& r) const;

private:
	int32_t m_index;
};

/*! \brief CPU SSE register abstraction.
 * \ingroup SW
 */
class XmmRegister
{
public:
	XmmRegister(int32_t index);

	int32_t getIndex() const;

private:
	int32_t m_index;
};

/*! \brief SSE memory pointer abstraction.
 * \ingroup SW
 */
class XmmWordPtr
{
public:
	XmmWordPtr(const void* ptr);

	const void* getPtr() const;

private:
	const void* m_ptr;
};

/*! \brief Runtime x86 assembler.
 * \ingroup SW
 */
class Assembler
{
public:
	void mov(const Register& dst, uint32_t value);

	void mov(const Register& dst, const Register& src);

	void mov(const Register& dst, const Register& src, uint32_t offset);

	void and(const Register& dst, uint32_t value);

	void add(const Register& dst, int32_t value);

	void add(const Register& dst, const Register& src);

	void mul(const Register& dst, int32_t value);

	void test(const Register& dst, const Register& src);

	void push(uint32_t value);

	void push(const Register& src);

	void push(const Register& src, uint32_t base);

	void pop(const Register& dst);

	void push(const XmmRegister& src);

	void pop(const XmmRegister& dst);

	void lea(const Register& dst, const Register& src, uint32_t base);

	void call(void* addr);

	void andps(const XmmRegister& dst, const XmmRegister& src);

	void andps(const XmmRegister& dst, const XmmWordPtr& ptr);

	void xorps(const XmmRegister& dst, const XmmWordPtr& ptr);

	void xorps(const XmmRegister& dst, const XmmRegister& src);

	void minps(const XmmRegister& dst, const XmmWordPtr& ptr);

	void minps(const XmmRegister& dst, const XmmRegister& src);

	void maxps(const XmmRegister& dst, const XmmWordPtr& ptr);

	void maxps(const XmmRegister& dst, const XmmRegister& src);

	void movaps(const XmmRegister& dst, const XmmWordPtr& ptr);

	void movaps(const XmmRegister& dst, const XmmRegister& src);

	void movaps(const XmmRegister& dst, const Register& src, uint32_t base);

	void movaps(const Register& dst, uint32_t base, const XmmRegister& src);

	void movhlps(const XmmRegister& dst, const XmmRegister& src);

	void addps(const XmmRegister& dst, const XmmRegister& src);

	void subps(const XmmRegister& dst, const XmmRegister& src);

	void mulps(const XmmRegister& dst, const XmmRegister& src);

	void divps(const XmmRegister& dst, const XmmRegister& src);

	void sqrtps(const XmmRegister& dst, const XmmRegister& src);

	void shufps(const XmmRegister& dst, const XmmRegister& src, uint8_t shuffle);

	void cvtss2si(const Register& dst, const XmmRegister& src);

	void cvttss2si(const Register& dst, const XmmRegister& src);

	void cvttps2dq(const XmmRegister& dst, const XmmRegister& src);

	void cvtdq2ps(const XmmRegister& dst, const XmmRegister& src);

	void cmpeqps(const XmmRegister& dst, const XmmRegister& src);

	void cmpltps(const XmmRegister& dst, const XmmRegister& src);

	void cmpleps(const XmmRegister& dst, const XmmRegister& src);

	void cmpneqps(const XmmRegister& dst, const XmmRegister& src);

	void ret();

	void label(uint32_t id);

	void jmp(uint32_t id);

	void jz(uint32_t id);

	void fixup();

	uint32_t size() const;

	const std::vector< uint8_t >& get() const;

private:
	std::vector< uint8_t > m_tmp;
	std::map< uint32_t, uint32_t > m_labels;
	std::vector< uint32_t > m_jumps;

	void db(uint8_t b);

	void dd(uint32_t d);
};

/*!
 * \ingroup SW
 */
//@{

extern Register eax;
extern Register ecx;
extern Register edx;
extern Register ebx;
extern Register esi;
extern Register edi;
extern XmmRegister xmm0;
extern XmmRegister xmm1;
extern XmmRegister xmm2;
extern XmmRegister xmm3;
extern XmmRegister xmm4;
extern XmmRegister xmm5;
extern XmmRegister xmm6;
extern XmmRegister xmm7;

//@}

	}
}

#endif	// traktor_render_Assembler_H
