#ifndef traktor_flash_ActionVMImage2_H
#define traktor_flash_ActionVMImage2_H

#include "Core/Misc/AutoPtr.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

enum NamespaceInfoKind
{
	CONSTANT_Namespace = 0x08,
	CONSTANT_PackageNamespace = 0x16,
	CONSTANT_PackageInternalNs = 0x17,
	CONSTANT_ProtectedNamespace = 0x18,
	CONSTANT_ExplicitNamespace = 0x19,
	CONSTANT_StaticProtectedNs = 0x1a,
	CONSTANT_PrivateNs = 0x05
};

struct NamespaceInfo
{
	uint8_t kind;	//!< \sa NamespaceInfoKind
	uint32_t name;
};

struct NamespaceSetInfo
{
	uint32_t count;
	AutoArrayPtr< uint32_t > ns;
};

enum MultinameInfoKind
{
	CONSTANT_QName = 0x07,
	CONSTANT_QNameA = 0x0d,
	CONSTANT_RTQName = 0x0f,
	CONSTANT_RTQNameA = 0x10,
	CONSTANT_RTQNameL = 0x11,
	CONSTANT_RTQNameLA = 0x12,
	CONSTANT_Multiname = 0x09,
	CONSTANT_MultinameA = 0x0e,
	CONSTANT_MultinameL = 0x1b,
	CONSTANT_MultinameLA = 0x1c
};

struct MultinameInfo
{
	uint8_t kind;	//!< \sa MultinameInfoKind
	union
	{
		struct
		{
			uint32_t ns;
			uint32_t name;
		}
		qname;

		struct
		{
			uint32_t name;
		}
		rtqname;

		struct
		{
			uint32_t name;
			uint32_t nsset;
		}
		multiname;

		struct
		{
			uint32_t nsset;
		}
		multinameL;
	};
};

struct ConstantPool
{
	AutoArrayPtr< int32_t > s32;
	AutoArrayPtr< uint32_t > u32;
	AutoArrayPtr< double > doubles;
	AutoArrayPtr< std::wstring > strings;
	AutoArrayPtr< NamespaceInfo > namespaces;
	AutoArrayPtr< NamespaceSetInfo > nsset;
	AutoArrayPtr< MultinameInfo > multinames;
};

class ActionVMImage2 : public IActionVMImage
{
	T_RTTI_CLASS;

public:
	virtual void execute(ActionFrame* frame) const;

	virtual void serialize(ISerializer& s);

private:
	friend class ActionVM2;

	ConstantPool m_cpool;
};

	}
}

#endif	// traktor_flash_ActionVMImage2_H
