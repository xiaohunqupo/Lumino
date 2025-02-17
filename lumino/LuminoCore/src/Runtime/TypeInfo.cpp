﻿#include <LuminoCore/Runtime/TypeInfo.hpp>

namespace ln {

//==============================================================================
// PredefinedTypes

TypeInfo* PredefinedTypes::Bool;
TypeInfo* PredefinedTypes::Char;

TypeInfo* PredefinedTypes::Int8;
TypeInfo* PredefinedTypes::Int16;
TypeInfo* PredefinedTypes::Int32;
TypeInfo* PredefinedTypes::Int64;

TypeInfo* PredefinedTypes::UInt8;
TypeInfo* PredefinedTypes::UInt16;
TypeInfo* PredefinedTypes::UInt32;
TypeInfo* PredefinedTypes::UInt64;

TypeInfo* PredefinedTypes::Float;
TypeInfo* PredefinedTypes::Double;

TypeInfo* PredefinedTypes::String;
TypeInfo* PredefinedTypes::Object;
TypeInfo* PredefinedTypes::List;

//==============================================================================
// TypeInfo

TypeInfo::TypeInfo(const String& className, TypeInfo* baseType, TypeInfoClass typeClass)
	: m_name(className)
	, m_typeClass(typeClass)
	, m_baseType(baseType)
	, m_managedTypeInfoId(-1)
{
}

TypeInfo::TypeInfo(const String& className)
	: m_name(className)
	, m_typeClass(TypeInfoClass::Object)
	, m_baseType(nullptr)
	, m_managedTypeInfoId(-1)
{
}

TypeInfo::~TypeInfo()
{
}

} // namespace ln

