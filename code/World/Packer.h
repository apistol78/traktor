#pragma once

#include "Core/Object.h"

namespace traktor::world
{

class Packer : public Object
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

	void reset();

private:
	class PackerImpl* m_impl;
};

}
