#ifndef traktor_flash_AbcFile_H
#define traktor_flash_AbcFile_H

#include "Core/Misc/AutoPtr.h"

namespace traktor
{
	namespace flash
	{

struct ConstantPool;
class SwfReader;

/*! \brief
 * \ingroup Flash
 */
enum NamespaceInfoKind
{
	Nsik_CONSTANT_Namespace = 0x08,
	Nsik_CONSTANT_PackageNamespace = 0x16,
	Nsik_CONSTANT_PackageInternalNs = 0x17,
	Nsik_CONSTANT_ProtectedNamespace = 0x18,
	Nsik_CONSTANT_ExplicitNamespace = 0x19,
	Nsik_CONSTANT_StaticProtectedNs = 0x1a,
	Nsik_CONSTANT_PrivateNs = 0x05
};

/*! \brief
 * \ingroup Flash
 */
struct NamespaceInfo
{
	uint8_t kind;	//!< \sa NamespaceInfoKind
	uint32_t name;

	NamespaceInfo();

	bool load(SwfReader& swf);
};

/*! \brief
 * \ingroup Flash
 */
struct NamespaceSetInfo
{
	uint32_t count;
	AutoArrayPtr< uint32_t > ns;

	NamespaceSetInfo();

	bool load(SwfReader& swf);
};

/*! \brief
 * \ingroup Flash
 */
enum MultinameInfoKind
{
	Mnik_CONSTANT_QName = 0x07,
	Mnik_CONSTANT_QNameA = 0x0d,
	Mnik_CONSTANT_RTQName = 0x0f,
	Mnik_CONSTANT_RTQNameA = 0x10,
	Mnik_CONSTANT_RTQNameL = 0x11,
	Mnik_CONSTANT_RTQNameLA = 0x12,
	Mnik_CONSTANT_Multiname = 0x09,
	Mnik_CONSTANT_MultinameA = 0x0e,
	Mnik_CONSTANT_MultinameL = 0x1b,
	Mnik_CONSTANT_MultinameLA = 0x1c
};

/*! \brief
 * \ingroup Flash
 */
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
	}
	data;

	MultinameInfo();

	bool load(SwfReader& swf);

	void dump(const ConstantPool& cpool) const;
};

/*! \brief
 * \ingroup Flash
 */
struct ConstantPool
{
	AutoArrayPtr< int32_t > s32;
	AutoArrayPtr< uint32_t > u32;
	AutoArrayPtr< double > doubles;
	AutoArrayPtr< std::wstring > strings;
	AutoArrayPtr< NamespaceInfo > namespaces;
	AutoArrayPtr< NamespaceSetInfo > nsset;
	AutoArrayPtr< MultinameInfo > multinames;

	bool load(SwfReader& swf);
};

/*! \brief
 * \ingroup Flash
 */
enum OptionDetailKind
{
	Odk_CONSTANT_Int = 0x03,
	Odk_CONSTANT_UInt = 0x04,
	Odk_CONSTANT_Double = 0x06,
	Odk_CONSTANT_Utf8 = 0x01,
	Odk_CONSTANT_True = 0x0b,
	Odk_CONSTANT_False = 0x0a,
	Odk_CONSTANT_Null = 0x0c,
	Odk_CONSTANT_Undefined = 0x00,
	Odk_CONSTANT_Namespace = 0x08,
	Odk_CONSTANT_PackageNamespace = 0x16,
	Odk_CONSTANT_PackageInternalNs = 0x17,
	Odk_CONSTANT_ProtectedNamespace = 0x18,
	Odk_CONSTANT_ExplicitNamespace = 0x19,
	Odk_CONSTANT_StaticProtectedNs = 0x1a,
	Odk_CONSTANT_PrivateNs = 0x05
};

/*! \brief
 * \ingroup Flash
 */
struct OptionDetail
{
	uint32_t val;
	uint8_t kind;	//!< \sa OptionDetailKind

	OptionDetail();

	bool load(SwfReader& swf);

	void dump(const ConstantPool& cpool) const;
};

/*! \brief
 * \ingroup Flash
 */
struct OptionInfo
{
	uint32_t optionCount;
	AutoArrayPtr< OptionDetail > options;

	OptionInfo();

	bool load(SwfReader& swf);

	void dump(const ConstantPool& cpool) const;
};

/*! \brief
 * \ingroup Flash
 */
struct ParamInfo
{
	AutoArrayPtr< uint32_t > names;

	bool load(SwfReader& swf, uint32_t paramCount);

	void dump(const ConstantPool& cpool, uint32_t paramCount) const;
};

/*! \brief
 * \ingroup Flash
 */
enum MethodInfoFlags
{
	Mif_NEED_ARGUMENTS = 0x01,
	Mif_NEED_ACTIVATION = 0x02,
	Mif_NEED_REST = 0x04,
	Mif_HAS_OPTIONAL = 0x08,
	Mif_SET_DXNS = 0x40,
	Mif_HAS_PARAM_NAMES = 0x80
};

/*! \brief
 * \ingroup Flash
 */
struct MethodInfo
{
	uint32_t paramCount;
	uint32_t returnType;
	AutoArrayPtr< uint32_t > paramTypes;
	uint32_t name;
	uint8_t flags;
	OptionInfo options;
	ParamInfo paramNames;

	MethodInfo();

	bool load(SwfReader& swf);

	void dump(const ConstantPool& cpool) const;
};

/*! \brief
 * \ingroup Flash
 */
struct ItemInfo
{
	uint32_t key;
	uint32_t value;

	ItemInfo();

	bool load(SwfReader& swf);

	void dump(const ConstantPool& cpool) const;
};

/*! \brief
 * \ingroup Flash
 */
struct MetaDataInfo
{
	uint32_t name;
	uint32_t itemCount;
	AutoArrayPtr< ItemInfo > items;

	MetaDataInfo();

	bool load(SwfReader& swf);

	void dump(const ConstantPool& cpool) const;
};

/*! \brief
 * \ingroup Flash
 */
enum TraitsInfoKind
{
	Tik_Trait_Slot = 0,
	Tik_Trait_Method = 1,
	Tik_Trait_Getter = 2,
	Tik_Trait_Setter = 3,
	Tik_Trait_Class = 4,
	Tik_Trait_Function = 5,
	Tik_Trait_Const = 6
};

/*! \brief
 * \ingroup Flash
 */
enum TraitsInfoAttribute
{
	Tia_ATTR_Final = 0x1,
	Tia_ATTR_Override = 0x2,
	Tia_ATTR_MetaData = 0x4
};

/*! \brief
 * \ingroup Flash
 */
struct TraitsInfo
{
	uint32_t name;
	uint8_t kind;		//!< [attribute:4][kind:4]
	union
	{
		struct
		{
			uint32_t slotId;
			uint32_t typeName;
			uint32_t vindex;
			uint8_t vkind;
		}
		slot;

		struct
		{
			uint32_t slotId;
			uint32_t classIndex;
		}
		clazz;

		struct
		{
			uint32_t slotId;
			uint32_t functionIndex;
		}
		function;

		struct
		{
			uint32_t dispId;
			uint32_t methodIndex;
		}
		method;
	}
	data;
	uint32_t metaDataCount;
	AutoArrayPtr< uint32_t > metaData;

	TraitsInfo();

	bool load(SwfReader& swf);

	void dump(const ConstantPool& cpool) const;
};

/*! \brief
 * \ingroup Flash
 */
enum InstanceInfoFlags
{
	Iif_CONSTANT_ClassSealed = 0x01,
	Iif_CONSTANT_ClassFinal = 0x02,
	Iif_CONSTANT_ClassInterface = 0x04,
	Iif_CONSTANT_ClassProtectedNs = 0x08
};

/*! \brief
 * \ingroup Flash
 */
struct InstanceInfo
{
	uint32_t name;
	uint32_t superName;
	uint8_t flags;
	uint32_t protectedNs;
	uint32_t interfaceCount;
	AutoArrayPtr< uint32_t > interfaces;
	uint32_t iinit;
	uint32_t traitsCount;
	AutoArrayPtr< TraitsInfo > traits;

	InstanceInfo();

	bool load(SwfReader& swf);

	void dump(const ConstantPool& cpool) const;
};

/*! \brief
 * \ingroup Flash
 */
struct ClassInfo
{
	uint32_t cinit;
	uint32_t traitsCount;
	AutoArrayPtr< TraitsInfo > traits;

	ClassInfo();

	bool load(SwfReader& swf);

	void dump(const ConstantPool& cpool) const;
};

/*! \brief
 * \ingroup Flash
 */
struct ScriptInfo
{
	uint32_t init;
	uint32_t traitsCount;
	AutoArrayPtr< TraitsInfo > traits;

	ScriptInfo();

	bool load(SwfReader& swf);

	void dump(const ConstantPool& cpool) const;
};

/*! \brief
 * \ingroup Flash
 */
struct ExceptionInfo
{
	uint32_t from;
	uint32_t to;
	uint32_t target;
	uint32_t exceptionType;
	uint32_t varName;

	ExceptionInfo();

	bool load(SwfReader& swf);

	void dump(const ConstantPool& cpool) const;
};

/*! \brief
 * \ingroup Flash
 */
struct MethodBodyInfo
{
	uint32_t method;
	uint32_t maxStack;
	uint32_t localCount;
	uint32_t initScopeDepth;
	uint32_t maxScopeDepth;
	uint32_t codeLength;
	AutoArrayPtr< uint8_t > code;
	uint32_t exceptionCount;
	AutoArrayPtr< ExceptionInfo > exceptions;
	uint32_t traitsCount;
	AutoArrayPtr< TraitsInfo > traits;

	MethodBodyInfo();

	bool load(SwfReader& swf);

	void dump(const ConstantPool& cpool) const;
};

/*! \brief
 * \ingroup Flash
 */
struct AbcFile
{
	uint16_t minorVersion;
	uint16_t majorVersion;
	ConstantPool cpool;
	uint32_t methodCount;
	AutoArrayPtr< MethodInfo > methods;
	uint32_t metaDataCount;
	AutoArrayPtr< MetaDataInfo > metaData;
	uint32_t classCount;
	AutoArrayPtr< InstanceInfo > instances;
	AutoArrayPtr< ClassInfo > classes;
	uint32_t scriptsCount;
	AutoArrayPtr< ScriptInfo > scripts;
	uint32_t methodBodyCount;
	AutoArrayPtr< MethodBodyInfo > methodBodies;

	AbcFile();

	bool load(SwfReader& swf);

	void dump() const;
};

	}
}

#endif	// traktor_flash_AbcFile_H
