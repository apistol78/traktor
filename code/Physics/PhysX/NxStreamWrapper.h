#ifndef traktor_physics_NxStreamWrapper_H
#define traktor_physics_NxStreamWrapper_H

#include "Core/Io/Stream.h"

namespace traktor
{
	namespace physics
	{

/*! \brief Wrapping Traktor Stream as a PhysX NxStream.
 * \ingroup PhysX
 */
class NxStreamWrapper : public NxStream
{
public:
	NxStreamWrapper(Stream& stream)
	:	m_stream(stream)
	{
	}

	virtual NxU8 readByte() const {
		return read< NxU8 >();
	}

	virtual NxU16 readWord() const {
		return read< NxU16 >();
	}

	virtual NxU32 readDword() const {
		return read< NxU32 >();
	}

	virtual NxF32 readFloat() const {
		return read< NxF32 >();
	}

	virtual NxF64 readDouble() const {
		return read< NxF64 >();
	}

	virtual void readBuffer(void* buffer, NxU32 size) const {
		m_stream.read(buffer, size);
	}

	virtual NxStream& storeByte(NxU8 b) {
		return write(b);
	}

	virtual NxStream& storeWord(NxU16 w) {
		return write(w);
	}

	virtual NxStream& storeDword(NxU32 d) {
		return write(d);
	}

	virtual NxStream& storeFloat(NxF32 f) {
		return write(f);
	}

	virtual NxStream& storeDouble(NxF64 d) {
		return write(d);
	}

	virtual NxStream& storeBuffer(const void* buffer, NxU32 size)
	{
		m_stream.write(buffer, size);
		return *this;
	}

private:
	mutable Stream& m_stream;

	template < typename T >
	T read() const
	{
		T v;
		m_stream.read(&v, sizeof(v));
		return v;
	}

	template < typename T >
	NxStream& write(T v)
	{
		m_stream.write(&v, sizeof(v));
		return *this;
	}
};

	}
}

#endif	// traktor_physics_NxStreamWrapper_H
