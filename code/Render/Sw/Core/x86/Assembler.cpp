/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Sw/Core/x86/Assembler.h"
#include "Core/Config.h"

namespace traktor
{
	namespace render
	{

Register::Register(int32_t index)
:	m_index(index)
{
}

int32_t Register::getIndex() const
{
	return m_index;
}

bool Register::operator == (const Register& r) const
{
	return r.m_index == m_index;
}

XmmRegister::XmmRegister(int32_t index)
:	m_index(index)
{
}

int32_t XmmRegister::getIndex() const
{
	return m_index;
}

XmmWordPtr::XmmWordPtr(const void* ptr)
:	m_ptr(ptr)
{
}

const void* XmmWordPtr::getPtr() const
{
	return m_ptr;
}

void Assembler::mov(const Register& dst, uint32_t value)
{
	db(0xb8 + dst.getIndex());
	dd(value);
}

void Assembler::mov(const Register& dst, const Register& src)
{
	db(0x8b);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::mov(const Register& dst, const Register& src, uint32_t offset)
{
	db(0x8b);
	db(0x80 + (dst.getIndex() << 3) + src.getIndex());
	dd(offset);
}

void Assembler::and(const Register& dst, uint32_t value)
{
	if (dst == eax)
		db(0x25);
	else
	{
		db(0x81);
		db(0xe0 + dst.getIndex());
	}
	dd(value);
}

void Assembler::add(const Register& dst, int32_t value)
{
	if (dst == eax)
		db(0x05);
	else
	{
		db(0x81);
		db(0xc0 + dst.getIndex());
	}
	dd(value);
}

void Assembler::add(const Register& dst, const Register& src)
{
	db(0x03);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::mul(const Register& dst, int32_t value)
{
	T_ASSERT (value >= 0 && value < 256);
	db(0x6b);
	db(0xc0 + (dst.getIndex() << 3) + dst.getIndex());
	db(uint8_t(value));
}

void Assembler::test(const Register& dst, const Register& src)
{
	db(0x85);
	db(0xc0 + (dst.getIndex() << 3) + dst.getIndex());
}

void Assembler::push(uint32_t value)
{
	db(0x68);
	dd(value);
}

void Assembler::push(const Register& src)
{
	db(0x50 + src.getIndex());
}

void Assembler::push(const Register& src, uint32_t base)
{
	db(0xff);
	db(0xb0 + src.getIndex());
	dd(base);
}

void Assembler::pop(const Register& dst)
{
	db(0x58 + dst.getIndex());
}

void Assembler::push(const XmmRegister& src)
{
	db(0x83);	// sub esp, 8
	db(0xec);
	db(0x08);

	db(0x0f);	// movups xmmword ptr [esp], xmm?
	db(0x11);
	db(0x0c + (src.getIndex() << 3));
	db(0x24);
}

void Assembler::pop(const XmmRegister& dst)
{
	db(0x0f);	// movups xmm?, xmmword ptr [esp]
	db(0x10);
	db(0x0c + (dst.getIndex() << 3));
	db(0x24);

	db(0x83);	// add esp, 8
	db(0xc4);
	db(0x08);
}

void Assembler::lea(const Register& dst, const Register& src, uint32_t base)
{
	db(0x8b);
	db(0x80 + (dst.getIndex() << 3) + src.getIndex());
	dd(base);
}

void Assembler::call(void* addr)
{
	db(0xb8);
	dd(uint32_t(addr));
	db(0xff);
	db(0xd0);
}

void Assembler::andps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0x54);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::andps(const XmmRegister& dst, const XmmWordPtr& ptr)
{
	db(0x0f);
	db(0x54);
	db(0x05 + (dst.getIndex() << 3));
	dd(uint32_t(ptr.getPtr()));
}

void Assembler::xorps(const XmmRegister& dst, const XmmWordPtr& ptr)
{
	db(0x0f);
	db(0x57);
	db(0x05 + (dst.getIndex() << 3));
	dd(uint32_t(ptr.getPtr()));
}

void Assembler::xorps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0x57);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::minps(const XmmRegister& dst, const XmmWordPtr& ptr)
{
	db(0x0f);
	db(0x5d);
	db(0x05 + (dst.getIndex() << 3));
	dd(uint32_t(ptr.getPtr()));
}

void Assembler::minps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0x5d);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::maxps(const XmmRegister& dst, const XmmWordPtr& ptr)
{
	db(0x0f);
	db(0x5f);
	db(0x05 + (dst.getIndex() << 3));
	dd(uint32_t(ptr.getPtr()));
}

void Assembler::maxps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0x5f);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::movaps(const XmmRegister& dst, const XmmWordPtr& ptr)
{
	db(0x0f);
	db(0x28);
	db(0x05 + (dst.getIndex() << 3));
	dd(uint32_t(ptr.getPtr()));
}

void Assembler::movaps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0x28);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::movaps(const XmmRegister& dst, const Register& src, uint32_t base)
{
	db(0x0f);
	db(0x28);	// movaps Vo,Wo
	db(0x80 + (dst.getIndex() << 3) + src.getIndex());
	dd(base);
}

void Assembler::movaps(const Register& dst, uint32_t base, const XmmRegister& src)
{
	db(0x0f);
	db(0x29);	// movaps Wo,Vo
	db(0x80 + (src.getIndex() << 3) + dst.getIndex());
	dd(base);
}

void Assembler::movhlps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0x12);
	db(0xc0 + (src.getIndex() << 3) + dst.getIndex());
}

void Assembler::addps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0x58);	// addps Vo, Wo
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::subps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0x5c);	// subps Vo, Wo
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::mulps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0x59);	// mulps Vo, Wo
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::divps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0x5e);	// divps Vo, Wo
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::sqrtps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0x51);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::shufps(const XmmRegister& dst, const XmmRegister& src, unsigned char shuffle)
{
	db(0x0f);
	db(0xc6);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
	db(shuffle);
}

void Assembler::cvtss2si(const Register& dst, const XmmRegister& src)
{
	db(0xf3);
	db(0x0f);
	db(0x2d);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::cvttss2si(const Register& dst, const XmmRegister& src)
{
	db(0xf3);
	db(0x0f);
	db(0x2c);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::cvttps2dq(const XmmRegister& dst, const XmmRegister& src)
{
	db(0xf3);
	db(0x0f);
	db(0x5b);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::cvtdq2ps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0x5b);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
}

void Assembler::cmpeqps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0xc2);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
	db(0x00);
}

void Assembler::cmpltps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0xc2);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
	db(0x01);
}

void Assembler::cmpleps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0xc2);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
	db(0x02);
}

void Assembler::cmpneqps(const XmmRegister& dst, const XmmRegister& src)
{
	db(0x0f);
	db(0xc2);
	db(0xc0 + (dst.getIndex() << 3) + src.getIndex());
	db(0x04);
}

void Assembler::ret()
{
	db(0xc3);
}

void Assembler::label(uint32_t id)
{
	m_labels[id] = uint32_t(m_tmp.size());
}

void Assembler::jmp(uint32_t id)
{
	m_jumps.push_back(uint32_t(m_tmp.size()));
	db(0xe9);
	dd(id);
}

void Assembler::jz(uint32_t id)
{
	m_jumps.push_back(uint32_t(m_tmp.size()));
	db(0x0f);
	db(0x84);
	dd(id);
}

void Assembler::fixup()
{
	for (std::vector< uint32_t >::iterator i = m_jumps.begin(); i != m_jumps.end(); ++i)
	{
		uint8_t inst = m_tmp[*i];
		uint32_t* rel = reinterpret_cast< uint32_t* >(&m_tmp[*i + (inst == 0xe9 ? 1 : 2)]);

		std::map< uint32_t, uint32_t >::iterator j = m_labels.find(*rel);
		T_ASSERT (j != m_labels.end());

		int32_t offset = j->second - *i - (inst == 0xe9 ? 1 : 2) - 4;
		*rel = uint32_t(offset);
	}
}

uint32_t Assembler::size() const
{
	return uint32_t(m_tmp.size());
}

const std::vector< uint8_t >& Assembler::get() const
{
	return m_tmp;
}

void Assembler::db(uint8_t b)
{
	m_tmp.push_back(b);
}

void Assembler::dd(uint32_t d)
{
	uint8_t* b = reinterpret_cast< uint8_t* >(&d);
	m_tmp.insert(m_tmp.end(), &b[0], &b[4]);
}

Register eax(0);
Register ecx(1);
Register edx(2);
Register ebx(3);
Register esp(4);
Register ebp(5);
Register esi(6);
Register edi(7);
XmmRegister xmm0(0);
XmmRegister xmm1(1);
XmmRegister xmm2(2);
XmmRegister xmm3(3);
XmmRegister xmm4(4);
XmmRegister xmm5(5);
XmmRegister xmm6(6);
XmmRegister xmm7(7);

	}
}
