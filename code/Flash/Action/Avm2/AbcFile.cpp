/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/BitReader.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/TString.h"
#include "Flash/SwfReader.h"
#include "Flash/Action/Avm2/AbcFile.h"
#include "Flash/Action/Avm2/ActionOpcodes.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

std::wstring getQualifiedName(const ConstantPool& cpool, uint32_t name)
{
	const MultinameInfo& mn = cpool.multinames[name];

	switch (mn.kind)
	{
	case Mnik_CONSTANT_QName:
	case Mnik_CONSTANT_QNameA:
		{
			const NamespaceInfo& ns = cpool.namespaces[mn.data.qname.ns];
			return mbstows(cpool.strings[ns.name] + " :: " + cpool.strings[mn.data.qname.name]);
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

bool NamespaceInfo::load(SwfReader& swf)
{
	BitReader& br = swf.getBitReader();
	kind = br.readUnsigned(8);
	name = swf.readEncodedU30();
	return true;
}

bool NamespaceSetInfo::load(SwfReader& swf)
{
	uint32_t count = swf.readEncodedU30();
	ns.resize(count);
	for (uint32_t i = 0; i < count; ++i)
		ns[i] = swf.readEncodedU30();
	return true;
}

MultinameInfo::MultinameInfo()
:	kind(0)
{
}

bool MultinameInfo::load(SwfReader& swf)
{
	BitReader& br = swf.getBitReader();
	kind = br.readUnsigned(8);
	switch (kind)
	{
	case Mnik_CONSTANT_QName:
	case Mnik_CONSTANT_QNameA:
		{
			data.qname.ns = swf.readEncodedU30();
			data.qname.name = swf.readEncodedU30();
		}
		break;

	case Mnik_CONSTANT_RTQName:
	case Mnik_CONSTANT_RTQNameA:
		{
			data.rtqname.name = swf.readEncodedU30();
		}
		break;

	case Mnik_CONSTANT_RTQNameL:
	case Mnik_CONSTANT_RTQNameLA:
		break;

	case Mnik_CONSTANT_Multiname:
	case Mnik_CONSTANT_MultinameA:
		{
			data.multiname.name = swf.readEncodedU30();
			data.multiname.nsset = swf.readEncodedU30();
		}
		break;

	case Mnik_CONSTANT_MultinameL:
	case Mnik_CONSTANT_MultinameLA:
		{
			data.multinameL.nsset = swf.readEncodedU30();
		}
		break;

	default:
		return false;
	}
	return true;
}

bool ConstantPool::load(SwfReader& swf)
{
	BitReader& br = swf.getBitReader();

	uint32_t cpoolInt32Count = swf.readEncodedU30();
	s32.resize(std::max(cpoolInt32Count, 1U));
	s32[0] = 0;
	for (uint32_t i = 1; i < cpoolInt32Count; ++i)
		s32[i] = swf.readEncodedU32();

	uint32_t cpoolUInt32Count = swf.readEncodedU30();
	u32.resize(std::max(cpoolUInt32Count, 1U));
	u32[0] = 0;
	for (uint32_t i = 1; i < cpoolUInt32Count; ++i)
		u32[i] = swf.readEncodedS32();

	uint32_t cpoolDoubleCount = swf.readEncodedU30();
	doubles.resize(std::max(cpoolDoubleCount, 1U));
	doubles[0] = 0.0;
	for (uint32_t i = 1; i < cpoolDoubleCount; ++i)
	{
		uint64_t dv = br.readUnsigned(64);
		doubles[i] = *(double*)&dv;
	}

	uint32_t cpoolStringCount = swf.readEncodedU30();
	strings.resize(std::max(cpoolStringCount, 1U));
	for (uint32_t i = 1; i < cpoolStringCount; ++i)
	{
		uint32_t length = swf.readEncodedU30();

		AutoArrayPtr< uint8_t > data(new uint8_t [length]);
		for (uint32_t j = 0; j < length; ++j)
			data[j] = br.readUnsigned(8);
		
		strings[i] = std::string(&data[0], &data[length]);
	}

	uint32_t cpoolNamespaceCount = swf.readEncodedU30();
	namespaces.resize(std::max(cpoolNamespaceCount, 1U));
	for (uint32_t i = 1; i < cpoolNamespaceCount; ++i)
	{
		if (!namespaces[i].load(swf))
			return false;
	}

	uint32_t cpoolNsSetCount = swf.readEncodedU30();
	nsset.resize(std::max(cpoolNsSetCount, 1U));
	for (uint32_t i = 1; i < cpoolNsSetCount; ++i)
	{
		if (!nsset[i].load(swf))
			return false;
	}

	uint32_t cpoolMultinameCount = swf.readEncodedU30();
	multinames.resize(std::max(cpoolMultinameCount, 1U));
	for (uint32_t i = 1; i < cpoolMultinameCount; ++i)
	{
		if (!multinames[i].load(swf))
			return false;
	}

	return true;
}

OptionDetail::OptionDetail()
:	val(0)
,	kind(0)
{
}

bool OptionDetail::load(SwfReader& swf)
{
	BitReader& br = swf.getBitReader();
	val = swf.readEncodedU30();
	kind = br.readUnsigned(8);
	return true;
}

void OptionDetail::dump(const ConstantPool& cpool) const
{
	log::info << L"val = " << val << Endl;
	log::info << L"kind = " << uint32_t(kind) << Endl;
}

bool OptionInfo::load(SwfReader& swf)
{
	uint32_t optionCount = swf.readEncodedU30();
	options.resize(optionCount);
	for (uint32_t i = 0; i < optionCount; ++i)
	{
		if (!options[i].load(swf))
			return false;
	}
	return true;
}

void OptionInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"optionCount = " << options.size() << Endl;
	for (uint32_t i = 0; i < options.size(); ++i)
	{
		log::info << L"options[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		options[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}";
	}
}

bool ParamInfo::load(SwfReader& swf, uint32_t paramCount)
{
	names.resize(paramCount);
	for (uint32_t i = 0; i < paramCount; ++i)
		names[i] = swf.readEncodedU30();
	return true;
}

void ParamInfo::dump(const ConstantPool& cpool) const
{
	for (uint32_t i = 0; i < names.size(); ++i)
		log::info << L"names[" << i << L"] = " << mbstows(cpool.strings[names[i]]) << Endl;
}

MethodInfo::MethodInfo()
:	returnType(0)
,	name(0)
,	flags(0)
{
}

bool MethodInfo::load(SwfReader& swf)
{
	BitReader& br = swf.getBitReader();

	uint32_t paramCount = swf.readEncodedU30();
	returnType = swf.readEncodedU30();

	if (paramCount > 0)
	{
		paramTypes.resize(paramCount);
		for (uint32_t j = 0; j < paramCount; ++j)
			paramTypes[j] = swf.readEncodedU30();
	}

	name = swf.readEncodedU30();
	flags = br.readUnsigned(8);
		
	if (flags & Mif_HAS_OPTIONAL)
	{
		if (!options.load(swf))
			return false;
	}

	if (flags & Mif_HAS_PARAM_NAMES)
	{
		if (!paramNames.load(swf, paramCount))
			return false;
	}

	return true;
}

void MethodInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"returnType = " << returnType << Endl;
		
	for (uint32_t j = 0; j < paramTypes.size(); ++j)
		log::info << L"paramTypes[" << j << L"] = " << paramTypes[j] << Endl;

	log::info << L"name = " << mbstows(cpool.strings[name]) << Endl;
	log::info << L"flags = " << uint32_t(flags) << Endl;

	if (flags & Mif_HAS_OPTIONAL)
	{
		log::info << L"options = {" << Endl;
		log::info << IncreaseIndent;
		options.dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}

	if (flags & Mif_HAS_PARAM_NAMES)
	{
		log::info << L"paramNames = {" << Endl;
		log::info << IncreaseIndent;
		paramNames.dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}
}

ItemInfo::ItemInfo()
:	key(0)
,	value(0)
{
}

bool ItemInfo::load(SwfReader& swf)
{
	key = swf.readEncodedU30();
	value = swf.readEncodedU30();
	return true;
}

void ItemInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"key = " << key << Endl;
	log::info << L"value = " << value << Endl;
}

MetaDataInfo::MetaDataInfo()
:	name(0)
{
}

bool MetaDataInfo::load(SwfReader& swf)
{
	name = swf.readEncodedU30();
	uint32_t itemCount = swf.readEncodedU30();
	items.resize(itemCount);
	for (uint32_t j = 0; j < itemCount; ++j)
	{
		if (!items[j].load(swf))
			return false;
	}
	return true;
}

void MetaDataInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"name = " << name << Endl;
	for (uint32_t i = 0; i < items.size(); ++i)
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
{
}

bool TraitsInfo::load(SwfReader& swf)
{
	BitReader& br = swf.getBitReader();

	if ((name = swf.readEncodedU30()) == 0)
		return false;

	kind = br.readUnsigned(8);

	switch (kind & 7)
	{
	case Tik_Trait_Slot:
	case Tik_Trait_Const:
		{
			data.slot.slotId = swf.readEncodedU30();
			data.slot.typeName = swf.readEncodedU30();
			data.slot.vindex = swf.readEncodedU30();
			if (data.slot.vindex != 0)
				data.slot.vkind = br.readUnsigned(8);
			else
				data.slot.vkind = 0;
		}
		break;

	case Tik_Trait_Class:
		{
			data.clazz.slotId = swf.readEncodedU30();
			data.clazz.classIndex = swf.readEncodedU30();
		}
		break;

	case Tik_Trait_Function:
		{
			data.function.slotId = swf.readEncodedU30();
			data.function.functionIndex = swf.readEncodedU30();
		}
		break;

	case Tik_Trait_Method:
	case Tik_Trait_Getter:
	case Tik_Trait_Setter:
		{
			data.method.dispId = swf.readEncodedU30();
			data.method.methodIndex = swf.readEncodedU30();
		}
		break;

	default:
		return false;
	}

	if (((kind >> 4) & Tia_ATTR_MetaData) == Tia_ATTR_MetaData)
	{
		uint32_t metaDataCount = swf.readEncodedU30();
		metaData.resize(metaDataCount);
		for (uint32_t i = 0; i < metaDataCount; ++i)
			metaData[i] = swf.readEncodedU30();
	}

	return true;
}

void TraitsInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"name = " << getQualifiedName(cpool, name) << Endl;
	log::info << L"kind = " << uint32_t(kind) << Endl;
	for (uint32_t i = 0; i < metaData.size(); ++i)
		log::info << L"metaData[" << i << L"] = " << metaData[i] << Endl;
}

InstanceInfo::InstanceInfo()
:	name(0)
,	superName(0)
,	flags(0)
,	protectedNs(0)
,	iinit(0)
{
}

bool InstanceInfo::load(SwfReader& swf)
{
	BitReader& br = swf.getBitReader();

	name = swf.readEncodedU30();
	superName = swf.readEncodedU30();
	flags = br.readUnsigned(8);

	if ((flags & Iif_CONSTANT_ClassProtectedNs) != 0)
		protectedNs = swf.readEncodedU30();

	uint32_t interfaceCount = swf.readEncodedU30();
	interfaces.resize(interfaceCount);
	for (uint32_t i = 0; i < interfaceCount; ++i)
		interfaces[i] = swf.readEncodedU30();

	iinit = swf.readEncodedU30();

	uint32_t traitsCount = swf.readEncodedU30();
	traits.resize(traitsCount);
	for (uint32_t i = 0; i < traitsCount; ++i)
	{
		if (!traits[i].load(swf))
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
	for (uint32_t i = 0; i < interfaces.size(); ++i)
		log::info << L"interfaces[" << i << L"] = " << interfaces[i] << Endl;
	log::info << L"iinit = " << iinit << Endl;
	for (uint32_t i = 0; i < traits.size(); ++i)
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
{
}

bool ClassInfo::load(SwfReader& swf)
{
	cinit = swf.readEncodedU30();

	uint32_t traitsCount = swf.readEncodedU30();
	traits.resize(traitsCount);
	for (uint32_t i = 0; i < traitsCount; ++i)
	{
		if (!traits[i].load(swf))
			return false;
	}

	return true;
}

void ClassInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"cinit = " << cinit << Endl;
	for (uint32_t i = 0; i < traits.size(); ++i)
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
{
}

bool ScriptInfo::load(SwfReader& swf)
{
	init = swf.readEncodedU30();

	uint32_t traitsCount = swf.readEncodedU30();
	traits.resize(traitsCount);
	for (uint32_t i = 0; i < traitsCount; ++i)
	{
		if (!traits[i].load(swf))
			return false;
	}

	return true;
}

void ScriptInfo::dump(const ConstantPool& cpool) const
{
	log::info << L"init = " << init << Endl;
	for (uint32_t i = 0; i < traits.size(); ++i)
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

bool ExceptionInfo::load(SwfReader& swf)
{
	from = swf.readEncodedU30();
	to = swf.readEncodedU30();
	target = swf.readEncodedU30();
	exceptionType = swf.readEncodedU30();
	varName = swf.readEncodedU30();
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
{
}

bool MethodBodyInfo::load(SwfReader& swf)
{
	BitReader& br = swf.getBitReader();

	method = swf.readEncodedU30();
	maxStack = swf.readEncodedU30();
	localCount = swf.readEncodedU30();
	initScopeDepth = swf.readEncodedU30();
	maxScopeDepth = swf.readEncodedU30();

	uint32_t codeLength = swf.readEncodedU30();
	code.resize(codeLength);
	for (uint32_t i = 0; i < codeLength; ++i)
		code[i] = br.readUnsigned(8);

	uint32_t exceptionCount = swf.readEncodedU30();
	exceptions.resize(exceptionCount);
	for (uint32_t i = 0; i < exceptionCount; ++i)
	{
		if (!exceptions[i].load(swf))
			return false;
	}

	uint32_t traitsCount = swf.readEncodedU30();
	traits.resize(traitsCount);
	for (uint32_t i = 0; i < traitsCount; ++i)
	{
		if (!traits[i].load(swf))
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

	const uint8_t T_UNALIGNED * pc = code.c_ptr();
	for (uint32_t i = 0; i < code.size(); ++i)
	{
		uint8_t op = *pc++;
		const Avm2OpCodeInfo* opInfo = findOpCodeInfo(op);
		if (opInfo)
		{
			log::info << i << L": " << opInfo->name << Endl;
			pc += opInfo->width;
		}
		else
			log::info << i << L": INVALID" << Endl;
	}

	for (uint32_t i = 0; i < exceptions.size(); ++i)
	{
		log::info << L"exceptions[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		exceptions[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}

	for (uint32_t i = 0; i < traits.size(); ++i)
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

bool AbcFile::load(SwfReader& swf)
{
	minorVersion = swf.readU16BE();
	majorVersion = swf.readU16BE();

	if (!cpool.load(swf))
		return false;

	uint32_t methodCount = swf.readEncodedU30();
	methods.resize(methodCount);
	for (uint32_t i = 0; i < methodCount; ++i)
	{
		if (!methods[i].load(swf))
			return false;
	}

	uint32_t metaDataCount = swf.readEncodedU30();
	metaData.resize(metaDataCount);
	for (uint32_t i = 0; i < metaDataCount; ++i)
	{
		if (!metaData[i].load(swf))
			return false;
	}

	uint32_t classCount = swf.readEncodedU30();
	instances.resize(classCount);
	for (uint32_t i = 0; i < classCount; ++i)
	{
		if (!instances[i].load(swf))
			return false;
	}
	classes.resize(classCount);
	for (uint32_t i = 0; i < classCount; ++i)
	{
		if (!classes[i].load(swf))
			return false;
	}

	uint32_t scriptsCount = swf.readEncodedU30();
	scripts.resize(scriptsCount);
	for (uint32_t i = 0; i < scriptsCount; ++i)
	{
		if (!scripts[i].load(swf))
			return false;
	}

	uint32_t methodBodyCount = swf.readEncodedU30();
	methodBodies.resize(methodBodyCount);
	for (uint32_t i = 0; i < methodBodyCount; ++i)
	{
		if (!methodBodies[i].load(swf))
			return false;
	}

	return true;
}

void AbcFile::dump() const
{
	for (uint32_t i = 0; i < methods.size(); ++i)
	{
		log::info << L"methods[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		methods[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}

	for (uint32_t i = 0; i < metaData.size(); ++i)
	{
		log::info << L"metaData[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		metaData[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}

	for (uint32_t i = 0; i < instances.size(); ++i)
	{
		log::info << L"instances[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		instances[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}

	for (uint32_t i = 0; i < classes.size(); ++i)
	{
		log::info << L"classes[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		classes[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}

	for (uint32_t i = 0; i < scripts.size(); ++i)
	{
		log::info << L"scripts[" << i << L"] = {" << Endl;
		log::info << IncreaseIndent;
		scripts[i].dump(cpool);
		log::info << DecreaseIndent;
		log::info << L"}" << Endl;
	}

	for (uint32_t i = 0; i < methodBodies.size(); ++i)
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
