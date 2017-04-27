/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Editor/BvhParser/BvhDocument.h"
#include "Animation/Editor/BvhParser/BvhJoint.h"
#include "Core/Io/AnsiEncoding.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

bool parseGroup(StringReader& sr, int32_t& channelCount, BvhJoint* joint, const Vector4& jointModifier)
{
	std::wstring str;

	if (sr.readLine(str) <= 0 || trim(str) != L"{")
		return false;

	for (;;)
	{
		if (sr.readLine(str) <= 0)
			return false;

		str = trim(str);

		if (startsWith< std::wstring >(str, L"OFFSET"))
		{
			std::vector< float > fv;
			if (Split< std::wstring, float >::any(str.substr(7), L" ", fv) >= 3)
			{
				joint->setOffset(Vector4(
					fv[0],
					fv[1],
					fv[2]
				) * jointModifier);
			}
		}
		else if (startsWith< std::wstring >(str, L"CHANNELS"))
		{
			std::vector< std::wstring > sv;
			if (Split< std::wstring >::any(str.substr(9), L" ", sv) >= 1)
			{
				int32_t nchannels = parseString< int32_t >(sv[0]);
				if (nchannels != int32_t(sv.size() - 1))
					return false;

				joint->setChannelOffset(channelCount);
				channelCount += nchannels;

				for (int i = 0; i < nchannels; ++i)
					joint->addChannel(sv[i + 1]);
			}
		}
		else if (startsWith< std::wstring >(str, L"JOINT"))
		{
			Ref< BvhJoint > childJoint = new BvhJoint(str.substr(6));
			if (!parseGroup(sr, channelCount, childJoint, jointModifier))
				return false;
			joint->addChild(childJoint);
		}
		else if (str == L"End Site")
		{
			Ref< BvhJoint > childJoint = new BvhJoint(L"");
			if (!parseGroup(sr, channelCount, childJoint, jointModifier))
				return false;
			joint->addChild(childJoint);
		}
		else if (str == L"}")
			break;
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BvhDocument", BvhDocument, Object)

Ref< BvhDocument > BvhDocument::parse(IStream* stream, const Vector4& jointModifier)
{
	BufferedStream bs(stream);
	StringReader sr(&bs, new AnsiEncoding());
	std::wstring str;

	if (sr.readLine(str) <= 0 || str != L"HIERARCHY")
		return 0;
	if (sr.readLine(str) <= 0 || !startsWith< std::wstring >(str, L"ROOT "))
		return 0;

	Ref< BvhDocument > document = new BvhDocument();
	document->m_rootJoint = new BvhJoint(str.substr(5));

	int32_t channelCount = 0;
	if (!parseGroup(sr, channelCount, document->m_rootJoint, jointModifier))
		return 0;

	if (sr.readLine(str) <= 0 || str != L"MOTION")
		return 0;
	if (sr.readLine(str) <= 0 || !startsWith< std::wstring >(str, L"Frames:"))
		return document;
	if (sr.readLine(str) <= 0 || !startsWith< std::wstring >(str, L"Frame Time:"))
		return 0;

	document->m_frameTime = parseString< float >(trim(str.substr(11)));

	while (sr.readLine(str) >= 0)
	{
		std::vector< float > fv;
		Split< std::wstring, float >::any(str, L" ", fv);

		if (int32_t(fv.size()) != channelCount)
			return 0;

		document->m_channelValues.push_back(fv);
	}

	return document;
}

BvhJoint* BvhDocument::getRootJoint() const
{
	return m_rootJoint;
}

float BvhDocument::getFrameTime() const
{
	return m_frameTime;
}

const BvhDocument::cv_list_t& BvhDocument::getChannelValues() const
{
	return m_channelValues;
}

	}
}
