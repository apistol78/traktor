#include "Render/Dx9/TypesDx9.h"

namespace traktor
{
	namespace render
	{

void textureCopy(void* d, const void* s, uint32_t bytes, TextureFormat textureFormat)
{
	uint32_t blockSize = getTextureBlockSize(textureFormat);
	uint32_t blockCount = bytes / blockSize;

	switch (textureFormat)
	{
	case TfR8G8B8A8:	// D3DFMT_A8R8G8B8
		{
			T_ASSERT (blockSize == 4);

			const uint8_t* sp = static_cast< const uint8_t* >(s);
			uint8_t* dp = static_cast< uint8_t* >(d);

			for (uint32_t i = 0; i < blockCount; ++i)
			{
				dp[0] = sp[2];	// B
				dp[1] = sp[1];	// G
				dp[2] = sp[0];	// R
				dp[3] = sp[3];	// A
				dp += 4;
				sp += 4;
			}
		}
		break;

	case TfR16G16B16A16F:	// D3DFMT_A16B16G16R16F
		{
			T_ASSERT (blockSize == 8);

			const uint16_t* sp = static_cast< const uint16_t* >(s);
			uint16_t* dp = static_cast< uint16_t* >(d);

			for (uint32_t i = 0; i < blockCount; ++i)
			{
				dp[0] = sp[0];
				dp[1] = sp[1];
				dp[2] = sp[2];
				dp[3] = sp[3];
				dp += 4;
				sp += 4;
			}
		}
		break;

	case TfR32G32B32A32F:	// D3DFMT_A32B32G32R32F
		{
			T_ASSERT (blockSize == 16);

			const uint32_t* sp = static_cast< const uint32_t* >(s);
			uint32_t* dp = static_cast< uint32_t* >(d);

			for (uint32_t i = 0; i < blockCount; ++i)
			{
				dp[0] = sp[0];
				dp[1] = sp[1];
				dp[2] = sp[2];
				dp[3] = sp[3];
				dp += 4;
				sp += 4;
			}
		}
		break;

	default:
		memcpy(d, s, bytes);
		break;
	}
}

	}
}
