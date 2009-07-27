#include <map>
#include "Render/Types.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Singleton/Singleton.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class HandleRegistry : public Singleton
{
public:
	static HandleRegistry& getInstance()
	{
		static HandleRegistry* s_instance = 0;
		if (!s_instance)
		{
			s_instance = new HandleRegistry();
			SingletonManager::getInstance().add(s_instance);
		}
		return *s_instance;
	}

	handle_t getHandle(const std::wstring& name)
	{
		std::map< std::wstring, handle_t >::iterator i = m_handles.find(name);
		if (i != m_handles.end())
			return i->second;

		handle_t handle = m_nextUnusedHandle++;
		m_handles.insert(std::make_pair(name, handle));

		return handle;
	}

protected:
	virtual void destroy()
	{
		delete this;
	}

private:
	std::map< std::wstring, handle_t > m_handles;
	handle_t m_nextUnusedHandle;

	HandleRegistry()
	:	m_nextUnusedHandle(1)
	{
	}
};

		}

handle_t getParameterHandle(const std::wstring& name)
{
	return HandleRegistry::getInstance().getHandle(name);
}

std::wstring getDataUsageName(DataUsage usage)
{
	const wchar_t* c_names[] =
	{
		L"DuPosition",
		L"DuNormal",
		L"DuTangent",
		L"DuBinormal",
		L"DuColor",
		L"DuCustom"
	};
	return c_names[int(usage)];
}

std::wstring getDataTypeName(DataType dataType)
{
	const wchar_t* c_names[] =
	{
		L"DtFloat1",
		L"DtFloat2",
		L"DtFloat3",
		L"DtFloat4",
		L"DtByte4",
		L"DtByte4N",
		L"DtShort2",
		L"DtShort4",
		L"DtShort2N",
		L"DtShort4N",
		L"DtHalf2",
		L"DtHalf4"
	};
	return c_names[int(dataType)];
}

uint32_t getDataElementCount(DataType dataType)
{
	const uint32_t c_elementCounts[] = { 1, 2, 3, 4, 4, 4, 2, 4, 2, 4, 2, 4 };
	return c_elementCounts[int(dataType)];
}

std::wstring getTextureFormatName(TextureFormat format)
{
	const wchar_t* c_names[] =
	{
		L"TfInvalid",
		L"TfR8",
		L"TfR8G8B8A8",
		L"TfR16G16B16A16F",
		L"TfR32G32B32A32F",
		L"TfR16F",
		L"TfR32F",
		L"TfDXT1",
		L"TfDXT2",
		L"TfDXT3",
		L"TfDXT4",
		L"TfDXT5"
	};
	return c_names[int(format)];
}

uint32_t getTextureBlockSize(TextureFormat format)
{
	const uint32_t c_blockSizes[] = { 0, 1, 4, 8, 16, 2, 4, 8, 16, 16, 16, 16 };
	return c_blockSizes[int(format)];
}

uint32_t getTextureBlockDenom(TextureFormat format)
{
	const uint32_t c_blockDenoms[] = { 1, 1, 1, 1, 1, 1, 1, 4, 4, 4, 4, 4 };
	return c_blockDenoms[int(format)];
}

uint32_t getTextureRowPitch(TextureFormat format, uint32_t textureWidth)
{
	uint32_t blockDenom = getTextureBlockDenom(format);
	uint32_t blockWidth = (textureWidth + blockDenom - 1) / blockDenom;
	return getTextureBlockSize(format) * blockWidth;
}

uint32_t getTextureRowPitch(TextureFormat format, uint32_t textureWidth, uint32_t mipLevel)
{
	return getTextureRowPitch(format, textureWidth >> mipLevel);
}

uint32_t getTextureMipPitch(TextureFormat format, uint32_t textureWidth, uint32_t textureHeight)
{
	uint32_t blockDenom = getTextureBlockDenom(format);
	uint32_t blockWidth = (textureWidth + blockDenom - 1) / blockDenom;
	uint32_t blockHeight = (textureHeight + blockDenom - 1) / blockDenom;
	uint32_t blockCount = blockWidth * blockHeight;
	return getTextureBlockSize(format) * blockCount;
}

uint32_t getTextureMipPitch(TextureFormat format, uint32_t textureWidth, uint32_t textureHeight, uint32_t mipLevel)
{
	return getTextureMipPitch(format, textureWidth >> mipLevel, textureHeight >> mipLevel);
}

	}
}
