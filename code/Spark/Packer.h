#ifndef traktor_flash_Packer_H
#define traktor_flash_Packer_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class T_DLLCLASS Packer : public Object
{
	T_RTTI_CLASS;

public:
	struct Rectangle
	{
		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;
	};

	Packer(int32_t width, int32_t height);

	virtual ~Packer();

	bool insert(int32_t width, int32_t height, Rectangle& outRectangle);

private:
	class PackerImpl* m_impl;
};

	}
}

#endif	// traktor_flash_Packer_H

