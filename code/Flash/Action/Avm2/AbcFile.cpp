#pragma optimize( "", off )

#include "Core/Io/BitReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/TString.h"
#include "Flash/Action/Avm2/AbcFile.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

uint16_t readU16(BitReader& br)
{
	uint16_t v = br.readUnsigned(16);
#if defined(T_LITTLE_ENDIAN)
	swap8in32(v);
#endif
	return v;
}

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

std::wstring getQualifiedName(const ConstantPool& cpool, uint32_t name)
{
	const MultinameInfo& mn = cpool.multinames[name];

	switch (mn.kind)
	{
	case Mnik_CONSTANT_QName:
	case Mnik_CONSTANT_QNameA:
		{
			const NamespaceInfo& ns = cpool.namespaces[mn.data.qname.ns];
			return cpool.strings[ns.name] + L" :: " + cpool.strings[mn.data.qname.name];
		}
	}

	return L"";
}

		}

NamespaceInfo::NamespaceInfo()
:	kind(0)
,	name(0)
{
}

bool NamespaceInfo::load(BitReader& br)
{
	kind = br.readUnsigned(8);
	name = readU30(br);
	return true;
}

NamespaceSetInfo::NamespaceSetInfo()
:	count(0)
{
}

bool NamespaceSetInfo::load(BitReader& br)
{
	count = readU30(br);
	ns.reset(new uint32_t [count]);
	for (uint32_t i = 0; i < count; ++i)
		ns[i] = readU30(br);
	return true;
}

MultinameInfo::MultinameInfo()
:	kind(0)
{
}

bool MultinameInfo::load(BitReader& br)
{
	kind = br.readUnsigned(8);
	switch (kind)
	{
	case Mnik_CONSTANT_QName:
	case Mnik_CONSTANT_QNameA:
		{
			data.qname.ns = readU30(br);
			data.qname.name = readU30(br);
		}
		break;

	case Mnik_CONSTANT_RTQName:
	case Mnik_CONSTANT_RTQNameA:
		{
			data.rtqname.name = readU30(br);
		}
		break;

	case Mnik_CONSTANT_RTQNameL:
	case Mnik_CONSTANT_RTQNameLA:
		break;

	case Mnik_CONSTANT_Multiname:
	case Mnik_CONSTANT_MultinameA:
		{
			data.multiname.name = readU30(br);
			data.multiname.nsset = readU30(br);
		}
		break;

	case Mnik_CONSTANT_MultinameL:
	case Mnik_CONSTANT_MultinameLA:
		{
			data.multinameL.nsset = readU30(br);
		}
		break;

	default:
		return false;
	}
	return true;
}

bool ConstantPool::load(BitReader& br)
{
	uint32_t cpoolInt32Count = readU30(br);
	s32.reset(new int32_t [cpoolInt32Count]);
	s32[0] = 0;
	for (uint32_t i = 1; i < cpoolInt32Count; ++i)
		s32[i] = readU32(br);

	uint32_t cpoolUInt32Count = readU30(br);
	u32.reset(new uint32_t [cpoolUInt32Count]);
	u32[0] = 0;
	for (uint32_t i = 1; i < cpoolUInt32Count; ++i)
		u32[i] = readS32(br);

	uint32_t cpoolDoubleCount = readU30(br);
	doubles.reset(new double [cpoolDoubleCount]);
	doubles[0] = 0.0;
	for (uint32_t i = 1; i < cpoolDoubleCount; ++i)
	{
		uint64_t dv = br.readUnsigned(64);
		doubles[i] = *(double*)&dv;
	}

	uint32_t cpoolStringCount = readU30(br);
	strings.reset(new std::wstring [cpoolStringCount]);
	for (uint32_t i = 1; i < cpoolStringCount; ++i)
	{
		uint32_t length = readU30(br);

		AutoArrayPtr< uint8_t > data(new uint8_t [length]);
		for (uint32_t j = 0; j < length; ++j)
			data[j] = br.readUnsigned(8);
		
		strings[i] = mbstows(Utf8Encoding(), std::string(&data[0], &data[length]));
	}

	uint32_t cpoolNamespaceCount = readU30(br);
	namespaces.reset(new NamespaceInfo [cpoolNamespaceCount]);
	for (uint32_t i = 1; i < cpoolNamespaceCount; ++i)
	{
		if (!namespaces[i].load(br))
			return false;
	}

	uint32_t cpoolNsSetCount = readU30(br);
	nsset.reset(new NamespaceSetInfo [cpoolNsSetCount]);
	for (uint32_t i = 1; i < cpoolNsSetCount; ++i)
	{
		if (!nsset[i].load(br))
			return false;
	}

	uint32_t cpoolMultinameCount = readU30(br);
	multinames.reset(new MultinameInfo [cpoolMultinameCount]);
	for (uint32_t i = 1; i < cpoolMultinameCount; ++i)
	{
		if (!multinames[i].load(br))
			return false;
	}

	return true;
}

OptionDetail::OptionDetail()
:	val(0)
,	kind(0)
{
}

bool OptionDetail::load(BitReader& br)
{
	val = readU30(br);
	kind = br.readUnsigned(8);
	return true;
}

void OptionDetail::dump(const ConstantPool& cpool) const
{
	log::info << L"val = " << val << Endl;
	log::info << L"kind = " << uint32_t(kind) << Endl;
}

OptionInfo::OptionInfo()
:	optionCount(0)
{
}

bool OptionInfo::load(BitReader& br)
{
	optionCount = readU30(br);
	options.reset(new OptionDetail [optionCount]);
	for (uint32_t i = 0; i < optionCount; ++i)
	{
		if (!options[i].load(br))
			return false;
	}
	return true;
}

void OptionInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"optionCount = " << optionCount << Endl;
	for (uint32_t i = 0; i < optionCount; ++i)
	{
		log::info << L"options[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		options[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}";
	}
}

bool ParamInfo::load(BitReader& br, uint32_t paramCount)
{
	names.reset(new uint32_t [paramCount]);
	for (uint32_t i = 0; i < paramCount; ++i)
		names[i] = readU30(br);
	return true;
}

void ParamInfo::dump(const ConstantPool& cpool, uint32_t paramCount) const
{
	for (uint32_t i = 0; i < paramCount; ++i)
		log::info << L"names[" << i << L"] = " << cpool.strings[names[i]] << Endl;
}

MethodInfo::MethodInfo()
:	paramCount(0)
,	returnType(0)
,	name(0)
,	flags(0)
{
}

bool MethodInfo::load(BitReader& br)
{
	paramCount = readU30(br);
	returnType = readU30(br);

	if (paramCount > 0)
	{
		paramTypes.reset(new uint32_t [paramCount]);
		for (uint32_t j = 0; j < paramCount; ++j)
			paramTypes[j] = readU30(br);
	}

	name = readU30(br);
	flags = br.readUnsigned(8);
		
	if (flags & Mif_HAS_OPTIONAL)
	{
		if (!options.load(br))
			return false;
	}

	if (flags & Mif_HAS_PARAM_NAMES)
	{
		if (!paramNames.load(br, paramCount))
			return false;
	}

	return true;
}

void MethodInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"paramCount = " << paramCount << Endl;
	log::info << L"returnType = " << returnType << Endl;
		
	for (uint32_t j = 0; j < paramCount; ++j)
		log::info << L"paramTypes[" << j << L"] = " << paramTypes[j] << Endl;

	log::info << L"name = " << cpool.strings[name] << Endl;
	log::info << L"flags = " << uint32_t(flags) << Endl;

	log::info << L"options = {" << Endl;
	log::info << IncreaseIndent;
	options.dump(cpool);
	log::info << DecreaseIndent;
	log::info << L"}" << Endl;

	log::info << L"paramNames = {" << Endl;
	log::info << IncreaseIndent;
	paramNames.dump(cpool, paramCount);
	log::info << DecreaseIndent;
	log::info << L"}" << Endl;
}

ItemInfo::ItemInfo()
:	key(0)
,	value(0)
{
}

bool ItemInfo::load(BitReader& br)
{
	key = readU30(br);
	value = readU30(br);
	return true;
}

void ItemInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"key = " << key << Endl;
	log::info << L"value = " << value << Endl;
}

MetaDataInfo::MetaDataInfo()
:	name(0)
,	itemCount(0)
{
}

bool MetaDataInfo::load(BitReader& br)
{
	name = readU30(br);
	itemCount = readU30(br);
	items.reset(new ItemInfo [itemCount]);
	for (uint32_t j = 0; j < itemCount; ++j)
	{
		if (!items[j].load(br))
			return false;
	}
	return true;
}

void MetaDataInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"name = " << name << Endl;
	log::info << L"itemCount = " << itemCount << Endl;
	for (uint32_t i = 0; i < itemCount; ++i)
	{
		log::info << L"items[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		items[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}
}

TraitsInfo::TraitsInfo()
:	name(0)
,	kind(0)
,	metaDataCount(0)
{
}

bool TraitsInfo::load(BitReader& br)
{
	name = readU30(br);
	kind = br.readUnsigned(8);

	switch (kind & 7)
	{
	case Tik_Trait_Slot:
	case Tik_Trait_Const:
		{
			data.slot.slotId = readU30(br);
			data.slot.typeName = readU30(br);
			data.slot.vindex = readU30(br);
			data.slot.vkind = br.readUnsigned(8);
		}
		break;

	case Tik_Trait_Class:
		{
			data.clazz.slotId = readU30(br);
			data.clazz.classIndex = readU30(br);
		}
		break;

	case Tik_Trait_Function:
		{
			data.function.slotId = readU30(br);
			data.function.functionIndex = readU30(br);
		}
		break;

	case Tik_Trait_Method:
	case Tik_Trait_Getter:
	case Tik_Trait_Setter:
		{
			data.method.dispId = readU30(br);
			data.method.methodIndex = readU30(br);
		}
		break;

	default:
		return false;
	}

	if (((kind >> 4) & Tia_ATTR_MetaData) == Tia_ATTR_MetaData)
	{
		metaDataCount = readU30(br);
		metaData.reset(new uint32_t [metaDataCount]);
		for (uint32_t i = 0; i < metaDataCount; ++i)
			metaData[i] = readU30(br);
	}

	return true;
}

void TraitsInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"name = " << getQualifiedName(cpool, name) << Endl;
	log::info << L"kind = " << uint32_t(kind) << Endl;
	log::info << L"metaDataCount = " << metaDataCount << Endl;
	for (uint32_t i = 0; i < metaDataCount; ++i)
		log::info << L"metaData[" << i << L"] = " << metaData[i] << Endl;
}

InstanceInfo::InstanceInfo()
:	name(0)
,	superName(0)
,	flags(0)
,	protectedNs(0)
,	interfaceCount(0)
,	iinit(0)
,	traitsCount(0)
{
}

bool InstanceInfo::load(BitReader& br)
{
	name = readU30(br);
	superName = readU30(br);
	flags = br.readUnsigned(8);
	protectedNs = readU30(br);

	interfaceCount = readU30(br);
	interfaces.reset(new uint32_t [interfaceCount]);
	for (uint32_t i = 0; i < interfaceCount; ++i)
		interfaces[i] = readU30(br);

	iinit = readU30(br);

	traitsCount = readU30(br);
	traits.reset(new TraitsInfo [traitsCount]);
	for (uint32_t i = 0; i < traitsCount; ++i)
	{
		if (!traits[i].load(br))
			return false;
	}

	return true;
}

void InstanceInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"name = " << getQualifiedName(cpool, name) << Endl;
	log::info << L"superName = " << getQualifiedName(cpool, superName) << Endl;
	log::info << L"flags = " << flags << Endl;
	log::info << L"protectedNs = " << protectedNs << Endl;
	log::info << L"interfaceCount = " << interfaceCount << Endl;
	for (uint32_t i = 0; i < interfaceCount; ++i)
		log::info << L"interfaces[" << i << L"] = " << interfaces[i] << Endl;
	log::info << L"iinit = " << iinit << Endl;
	log::info << L"traitsCount = " << traitsCount << Endl;
	for (uint32_t i = 0; i < traitsCount; ++i)
	{
		log::info << L"traits[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		traits[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}
}

ClassInfo::ClassInfo()
:	cinit(0)
,	traitsCount(0)
{
}

bool ClassInfo::load(BitReader& br)
{
	cinit = readU30(br);

	traitsCount = readU30(br);
	traits.reset(new TraitsInfo [traitsCount]);
	for (uint32_t i = 0; i < traitsCount; ++i)
	{
		if (!traits[i].load(br))
			return false;
	}

	return true;
}

void ClassInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"cinit = " << cinit << Endl;
	log::info << L"traitsCount = " << traitsCount << Endl;
	for (uint32_t i = 0; i < traitsCount; ++i)
	{
		log::info << L"traits[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		traits[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}
}

ScriptInfo::ScriptInfo()
:	init(0)
,	traitsCount(0)
{
}

bool ScriptInfo::load(BitReader& br)
{
	init = readU30(br);

	traitsCount = readU30(br);
	traits.reset(new TraitsInfo [traitsCount]);
	for (uint32_t i = 0; i < traitsCount; ++i)
	{
		if (!traits[i].load(br))
			return false;
	}

	return true;
}

void ScriptInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"init = " << init << Endl;
	log::info << L"traitsCount = " << traitsCount << Endl;
	for (uint32_t i = 0; i < traitsCount; ++i)
	{
		log::info << L"traits[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		traits[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}
}

ExceptionInfo::ExceptionInfo()
:	from(0)
,	to(0)
,	target(0)
,	exceptionType(0)
,	varName(0)
{
}

bool ExceptionInfo::load(BitReader& br)
{
	from = readU30(br);
	to = readU30(br);
	target = readU30(br);
	exceptionType = readU30(br);
	varName = readU30(br);
	return true;
}

void ExceptionInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"from = " << from << Endl;
	log::info << L"to = " << to << Endl;
	log::info << L"target = " << target << Endl;
	log::info << L"exceptionType = " << exceptionType << Endl;
	log::info << L"varName = " << varName << Endl;
}

MethodBodyInfo::MethodBodyInfo()
:	method(0)
,	maxStack(0)
,	localCount(0)
,	initScopeDepth(0)
,	maxScopeDepth(0)
,	codeLength(0)
,	exceptionCount(0)
,	traitsCount(0)
{
}

bool MethodBodyInfo::load(BitReader& br)
{
	method = readU30(br);
	maxStack = readU30(br);
	localCount = readU30(br);
	initScopeDepth = readU30(br);
	maxScopeDepth = readU30(br);

	codeLength = readU30(br);
	code.reset(new uint8_t [codeLength]);
	for (uint32_t i = 0; i < codeLength; ++i)
		code[i] = br.readUnsigned(8);

	exceptionCount = readU30(br);
	exceptions.reset(new ExceptionInfo [exceptionCount]);
	for (uint32_t i = 0; i < exceptionCount; ++i)
	{
		if (!exceptions[i].load(br))
			return false;
	}

	traitsCount = readU30(br);
	traits.reset(new TraitsInfo [traitsCount]);
	for (uint32_t i = 0; i < traitsCount; ++i)
	{
		if (!traits[i].load(br))
			return false;
	}

	return true;
}

void MethodBodyInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"method = " << method << Endl;
	log::info << L"maxStack = " << maxStack << Endl;
	log::info << L"localCount = " << localCount << Endl;
	log::info << L"initScopeDepth = " << initScopeDepth << Endl;
	log::info << L"maxScopeDepth = " << maxScopeDepth << Endl;
	log::info << L"codeLength = " << codeLength << Endl;
	log::info << L"exceptionCount = " << exceptionCount << Endl;
	for (uint32_t i = 0; i < exceptionCount; ++i)
	{
		log::info << L"exceptions[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		exceptions[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}
	log::info << L"traitsCount = " << traitsCount << Endl;
	for (uint32_t i = 0; i < traitsCount; ++i)
	{
		log::info << L"traits[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		traits[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}
}

AbcFile::AbcFile()
:	minorVersion(0)
,	majorVersion(0)
{
}

bool AbcFile::load(BitReader& br)
{
	minorVersion = readU16(br);
	majorVersion = readU16(br);

	if (!cpool.load(br))
		return false;

	methodCount = readU30(br);
	methods.reset(new MethodInfo [methodCount]);
	for (uint32_t i = 0; i < methodCount; ++i)
	{
		if (!methods[i].load(br))
			return false;
	}

	metaDataCount = readU30(br);
	metaData.reset(new MetaDataInfo [metaDataCount]);
	for (uint32_t i = 0; i < metaDataCount; ++i)
	{
		if (!metaData[i].load(br))
			return false;
	}

	classCount = readU30(br);
	instances.reset(new InstanceInfo [classCount]);
	for (uint32_t i = 0; i < classCount; ++i)
	{
		if (!instances[i].load(br))
			return false;
	}
	classes.reset(new ClassInfo [classCount]);
	for (uint32_t i = 0; i < classCount; ++i)
	{
		if (!classes[i].load(br))
			return false;
	}

	scriptsCount = readU30(br);
	scripts.reset(new ScriptInfo [scriptsCount]);
	for (uint32_t i = 0; i < scriptsCount; ++i)
	{
		if (!scripts[i].load(br))
			return false;
	}

	methodBodyCount = readU30(br);
	methodBodies.reset(new MethodBodyInfo [methodBodyCount]);
	for (uint32_t i = 0; i < methodBodyCount; ++i)
	{
		if (!methodBodies[i].load(br))
			return false;
	}

	return true;
}

void AbcFile::dump() const
{
	log::info << L"methodCount = " << methodCount << Endl;
	for (uint32_t i = 0; i < methodCount; ++i)
	{
		log::info << L"methods[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		methods[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}

	log::info << L"metaDataCount = " << metaDataCount << Endl;
	for (uint32_t i = 0; i < metaDataCount; ++i)
	{
		log::info << L"metaData[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		metaData[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}

	log::info << L"classCount = " << classCount << Endl;
	for (uint32_t i = 0; i < classCount; ++i)
	{
		log::info << L"instances[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		instances[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}
	for (uint32_t i = 0; i < classCount; ++i)
	{
		log::info << L"classes[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		classes[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}

	log::info << L"scriptsCount = " << scriptsCount << Endl;
	for (uint32_t i = 0; i < scriptsCount; ++i)
	{
		log::info << L"scripts[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		scripts[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}

	log::info << L"methodBodyCount = " << methodBodyCount << Endl;
	for (uint32_t i = 0; i < methodBodyCount; ++i)
	{
		log::info << L"methodBodies[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		methodBodies[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}
}

	}
}
