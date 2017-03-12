#ifndef traktor_Reader_H
#define traktor_Reader_H

#include <string>
#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Primitive reader class.
 * \ingroup Core
 *
 * This class resolves endian issues automatically
 * and should therefor be used in the serialization process
 * to ensure non-endian dependent data files.
 * It will also keep the embedded stream in sync with the
 * current reading point, i.e. it will not buffer anything.
 */
class T_DLLCLASS Reader : public Object
{
	T_RTTI_CLASS;

public:
	Reader(IStream* stream);
	
	Reader& operator >> (bool& b);
	
	Reader& operator >> (int8_t& c);
	
	Reader& operator >> (uint8_t& uc);
	
	Reader& operator >> (int16_t& i);
	
	Reader& operator >> (uint16_t& ui);
	
	Reader& operator >> (int32_t& i);
	
	Reader& operator >> (uint32_t& ui);
	
	Reader& operator >> (int64_t& i);

	Reader& operator >> (uint64_t& ui);
	
	Reader& operator >> (float& f);
	
	Reader& operator >> (double& f);
	
	Reader& operator >> (std::wstring& s);
	
	int64_t read(void* block, int64_t nbytes);
	
	int64_t read(void* block, int64_t count, int64_t size);

	int64_t skip(int64_t nbytes);

private:
	Ref< IStream > m_stream;
};

}

#endif	// traktor_Reader_H
