#include "Core/Io/BitReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/TString.h"
#include "Flash/Action/Avm2/ActionVM2.h"
#include "Flash/Action/Avm2/ActionVMImage2.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

uint32_t readU30(BitReader& br)
{
	uint32_t out = 0;
	for (uint32_t i = 0; i < 5; ++i)
	{
		uint8_t v = br.readUnsigned(8);
		out |= (v & 0x7f) << (i * 7);
		if ((v & 0x80) == 0x00)
			break;
	}
	return out;
}

uint32_t readU32(BitReader& br)
{
	uint32_t out = 0;
	for (uint32_t i = 0; i < 5; ++i)
	{
		uint8_t v = br.readUnsigned(8);
		out |= (v & 0x7f) << (i * 7);
		if ((v & 0x80) == 0x00)
			break;
	}
	return out;
}

int32_t readS32(BitReader& br)
{
	int32_t out = 0;
	for (uint32_t i = 0; i < 5; ++i)
	{
		uint8_t v = br.readUnsigned(8);
		out |= (v & 0x7f) << (i * 7);
		if ((v & 0x80) == 0x00)
			break;
	}
	return out;
}

		}


T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionVM2", ActionVM2, IActionVM)

Ref< const IActionVMImage > ActionVM2::load(BitReader& br) const
{
	Ref< ActionVMImage2 > image = new ActionVMImage2();

	// Read ABC version.
	uint16_t minorVersion = br.readUnsigned(16);
	uint16_t majorVersion = br.readUnsigned(16);

	// Read constant pool values.
	uint32_t cpoolInt32Count = readU30(br);
	image->m_cpool.s32.reset(new int32_t [cpoolInt32Count]);
	for (uint32_t i = 1; i < cpoolInt32Count; ++i)
		image->m_cpool.s32[i] = readU32(br);

	uint32_t cpoolUInt32Count = readU30(br);
	image->m_cpool.u32.reset(new uint32_t [cpoolUInt32Count]);
	for (uint32_t i = 1; i < cpoolUInt32Count; ++i)
		image->m_cpool.u32[i] = readS32(br);

	uint32_t cpoolDoubleCount = readU30(br);
	image->m_cpool.doubles.reset(new double [cpoolDoubleCount]);
	for (uint32_t i = 1; i < cpoolDoubleCount; ++i)
	{
		uint64_t dv = br.readUnsigned(64);
		image->m_cpool.doubles[i] = *(double*)&dv;
	}

	uint32_t cpoolStringCount = readU30(br);
	image->m_cpool.strings.reset(new std::wstring [cpoolStringCount]);
	for (uint32_t i = 1; i < cpoolStringCount; ++i)
	{
		uint32_t length = readU30(br);

		AutoArrayPtr< uint8_t > data(new uint8_t [length]);
		for (uint32_t j = 0; j < length; ++j)
			data[i] = br.readUnsigned(8);
		
		image->m_cpool.strings[i] = mbstows(Utf8Encoding(), std::string(&data[0], &data[length]));
	}

	uint32_t cpoolNamespaceCount = readU30(br);
	image->m_cpool.namespaces.reset(new NamespaceInfo [cpoolNamespaceCount]);
	for (uint32_t i = 1; i < cpoolNamespaceCount; ++i)
	{
		image->m_cpool.namespaces[i].kind = br.readUnsigned(8);
		image->m_cpool.namespaces[i].name = readU30(br);
	}

	uint32_t cpoolNsSetCount = readU30(br);
	image->m_cpool.nsset.reset(new NamespaceSetInfo [cpoolNsSetCount]);
	for (uint32_t i = 1; i < cpoolNsSetCount; ++i)
	{
		image->m_cpool.nsset[i].count = readU30(br);
		image->m_cpool.nsset[i].ns.reset(new uint32_t [image->m_cpool.nsset[i].count]);
		for (uint32_t j = 0; j < image->m_cpool.nsset[i].count; ++j)
			image->m_cpool.nsset[i].ns[j] = readU30(br);
	}

	uint32_t cpoolMultinameCount = readU30(br);
	image->m_cpool.multinames.reset(new MultinameInfo [cpoolMultinameCount]);
	for (uint32_t i = 1; i < cpoolMultinameCount; ++i)
	{
		image->m_cpool.multinames[i].kind = br.readUnsigned(8);
		switch (image->m_cpool.multinames[i].kind)
		{
		case CONSTANT_QName:
		case CONSTANT_QNameA:
			{
				image->m_cpool.multinames[i].qname.ns = readU30(br);
				image->m_cpool.multinames[i].qname.name = readU30(br);
			}
			break;

		case CONSTANT_RTQName:
		case CONSTANT_RTQNameA:
			{
				image->m_cpool.multinames[i].rtqname.name = readU30(br);
			}
			break;

		case CONSTANT_RTQNameL:
		case CONSTANT_RTQNameLA:
			break;

		case CONSTANT_Multiname:
		case CONSTANT_MultinameA:
			{
				image->m_cpool.multinames[i].multiname.name = readU30(br);
				image->m_cpool.multinames[i].multiname.nsset = readU30(br);
			}
			break;

		case CONSTANT_MultinameL:
		case CONSTANT_MultinameLA:
			{
				image->m_cpool.multinames[i].multinameL.nsset = readU30(br);
			}
			break;

		default:
			return 0;
		}
	}

	// Read methods.
	uint32_t methodCount = readU30(br);

	// Read metadata.
	uint32_t metadataCount = readU30(br);

	// Read classes.
	uint32_t classCount = readU30(br);

	// Read scripts.
	uint32_t scriptCount = readU30(br);

	// Read method bodies.
	uint32_t methodBodyCount = readU30(br);

	return image;
}

	}
}
