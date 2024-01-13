#include "Config.h"
#include "parson.h"
#include "string.h"
#include "stdio.h"

#include "Leaks.h"

// C++ wrapper for JSON parser library "Parson"

Config::Config()
{
	vroot = json_value_init_object();
	root = json_value_get_object(vroot);
	needs_removal = true;
}

Config::Config(const char * string)
{
	if (string != nullptr)
	{
		vroot = json_parse_string(string);
		if (vroot != nullptr) {
			root = json_value_get_object(vroot);
			needs_removal = true;
		}
	}
}

Config::Config(JSON_Object* section) : root(section)
{}

Config::~Config()
{
	if (needs_removal == true)
		json_value_free(vroot);
}

bool Config::IsValid() const
{
	return root != nullptr;
}

size_t Config::Save(char** buf, const char* title_comment) const
{
	size_t written = json_serialization_size(vroot);
	*buf = new char[written];
	json_serialize_to_buffer(vroot, *buf, written);
	return written;
}

int Config::Size() const
{
	return (root) ? int(json_object_get_count(root)) : 0;
}

Config Config::GetSection(const char * section_name) const
{
	return Config(json_object_get_object(root, section_name));
}

Config Config::AddSection(const char * section_name)
{
	json_object_set_value(root, section_name, json_value_init_object());
	return GetSection(section_name);
}


JSON_Value * Config::FindValue(const char * field, int index) const
{
	if (index < 0)
		return json_object_get_value(root, field);

	JSON_Array* array = json_object_get_array(root, field);
	if (array != nullptr)
		return json_array_get_value(array, index);

	return nullptr;
}

bool Config::HasBool(const char *field, int index) const
{
	JSON_Value* value = FindValue(field, index);

	return value && json_value_get_type(value) == JSONBoolean;
}

bool Config::HasInt(const char *field, int index) const
{
	JSON_Value* value = FindValue(field, index);

	return value && json_value_get_type(value) == JSONNumber;
}

bool Config::HasUInt(const char *field, int index) const
{
	JSON_Value* value = FindValue(field, index);

	return value && json_value_get_type(value) == JSONNumber;
}

bool Config::HasUID(const char *field, int index) const
{
	JSON_Value* value = FindValue(field, index);

	return value && json_value_get_type(value) == JSONNumber;
}

bool Config::HasDouble(const char *field, int index) const
{
	JSON_Value* value = FindValue(field, index);

	return value && json_value_get_type(value) == JSONNumber;
}

bool Config::HasFloat(const char *field, int index) const
{
	JSON_Value* value = FindValue(field, index);

	return value && json_value_get_type(value) == JSONNumber;
}

bool Config::HasString(const char *field, int index) const
{
	JSON_Value* value = FindValue(field, index);

	return value && json_value_get_type(value) == JSONString;
}

bool Config::GetBool(const char * field, bool def, int index) const
{
	JSON_Value* value = FindValue(field, index);

	if (value && json_value_get_type(value) == JSONBoolean)
		return json_value_get_boolean(value) != 0;

	return def;
}

int Config::GetInt(const char * field, int def, int index) const
{
	JSON_Value* value = FindValue(field, index);

	if (value && json_value_get_type(value) == JSONNumber)
		return (int) json_value_get_number(value);

	return def;
}

uint Config::GetUInt(const char * field, uint def, int index) const
{
	JSON_Value* value = FindValue(field, index);

	if (value && json_value_get_type(value) == JSONNumber)
		return (uint) json_value_get_number(value);

	return def;
}

UID Config::GetUID(const char * field, UID def, int index) const
{
	JSON_Value* value = FindValue(field, index);

	if (value && json_value_get_type(value) == JSONNumber)
		return (UID) json_value_get_number(value);

	return def;
}

double Config::GetDouble(const char * field, double def, int index) const
{
	JSON_Value* value = FindValue(field, index);

	if (value && json_value_get_type(value) == JSONNumber)
		return json_value_get_number(value);

	return def;
}

float Config::GetFloat(const char * field, float def, int index) const
{
	JSON_Value* value = FindValue(field, index);

	if (value && json_value_get_type(value) == JSONNumber)
		return (float) json_value_get_number(value);

	return def;
}

const char* Config::GetString(const char * field, const char* def, int index) const
{
	JSON_Value* value = FindValue(field, index);

	if (value && json_value_get_type(value) == JSONString)
		return json_value_get_string(value);

	return def;
}

Config Config::GetArray(const char * field, int index) const
{
	JSON_Array* array = json_object_get_array(root, field);
	if (array != nullptr)
		return Config(json_array_get_object(array, index));
	return Config((JSON_Object*) nullptr);
}

int Config::GetArrayCount(const char * field) const
{
	int ret = 0;
	JSON_Array* array = json_object_get_array(root, field);
	if (array != nullptr)
		ret = int(json_array_get_count(array));
	return ret;
}

bool Config::AddBool(const char * field, bool value)
{
	return json_object_set_boolean(root, field, (value) ? 1 : 0) == JSONSuccess;
}

bool Config::AddInt(const char * field, int value)
{
	return json_object_set_number(root, field, (double) value) == JSONSuccess;
}

bool Config::AddUInt(const char * field, uint value)
{
	return json_object_set_number(root, field, (double) value) == JSONSuccess;
}

bool Config::AddUID(const char * field, UID value)
{
	return json_object_set_number(root, field, (double) value) == JSONSuccess;
}

bool Config::AddDouble(const char * field, double value)
{
	return json_object_set_number(root, field, value) == JSONSuccess;
}

bool Config::AddFloat(const char * field, float value)
{
	return json_object_set_number(root, field, (float) value) == JSONSuccess;
}

bool Config::AddString(const char * field, const char * string)
{
	return json_object_set_string(root, field, string) == JSONSuccess;
}

bool Config::AddArray(const char* array_name)
{
	JSON_Value* va = json_value_init_array();
	array = json_value_get_array(va);

	return json_object_set_value(root, array_name, va) == JSONSuccess;
}

bool Config::AddArrayEntry(const Config & config)
{
	if (array != nullptr)
		return json_array_append_value(array, json_value_deep_copy(config.vroot)) == JSONSuccess;

	return false;
}

bool Config::AddArrayBool(const char * field, const bool * values, int size)
{
	if (values != nullptr && size > 0)
	{
		JSON_Value* va = json_value_init_array();
		array = json_value_get_array(va);
		json_object_set_value(root, field, va);

		for(int i=0; i < size; ++i)
			json_array_append_boolean(array, values[i]);
		return true;
	}
	return false;
}

bool Config::AddArrayInt(const char * field, const int * values, int size)
{
	if (values != nullptr && size > 0)
	{
		JSON_Value* va = json_value_init_array();
		array = json_value_get_array(va);
		json_object_set_value(root, field, va);

		for(int i=0; i < size; ++i)
			json_array_append_number(array, values[i]);
		return true;
	}
	return false;
}

bool Config::AddArrayUInt(const char * field, const uint * values, int size)
{
	if (values != nullptr && size > 0)
	{
		JSON_Value* va = json_value_init_array();
		array = json_value_get_array(va);
		json_object_set_value(root, field, va);

		for(int i=0; i < size; ++i)
			json_array_append_number(array, values[i]);
		return true;
	}
	return false;
}

bool Config::AddArrayUID(const char * field, const UID * values, int size)
{
	if (values != nullptr && size > 0)
	{
		JSON_Value* va = json_value_init_array();
		array = json_value_get_array(va);
		json_object_set_value(root, field, va);

		for(int i=0; i < size; ++i)
			json_array_append_number(array, (double)values[i]);
		return true;
	}
	return false;
}

bool Config::AddArrayFloat(const char * field, const float * values, int size)
{
	if (values != nullptr && size > 0)
	{
		JSON_Value* va = json_value_init_array();
		array = json_value_get_array(va);
		json_object_set_value(root, field, va);

		for(int i=0; i < size; ++i)
			json_array_append_number(array, values[i]);
		return true;
	}
	return false;
}

bool Config::AddArrayFloat3(const char* field, const float3* values, int size)
{
	if (values != nullptr && size > 0)
	{
		JSON_Value* va = json_value_init_array();
		array = json_value_get_array(va);
		json_object_set_value(root, field, va);

		for(int i=0; i < size; ++i)
        {
			json_array_append_number(array, values[i].x);
			json_array_append_number(array, values[i].y);
			json_array_append_number(array, values[i].z);
        }
		return true;
	}
	return false;
}

bool Config::AddArrayString(const char * field, const char ** values, int size)
{
	if (values != nullptr && size > 0)
	{
		JSON_Value* va = json_value_init_array();
		array = json_value_get_array(va);
		json_object_set_value(root, field, va);

		for(int i=0; i < size; ++i)
			json_array_append_string(array, values[i]);
		return true;
	}
	return false;
}

bool Config::AddFloat2(const char* field, const float2& value)
{
	return AddArrayFloat(field, &value.x, 2);
}

float2 Config::GetFloat2(const char* field, const float2& def /*= float2::zero*/)
{
	return float2(GetFloat(field, def.x, 0), GetFloat(field, def.y, 1));
}

bool Config::AddFloat3(const char * field, const float3 & value)
{
	return AddArrayFloat(field, &value.x, 3);
}

float3 Config::GetFloat3(const char * field, const float3 & def)
{
	return float3(
		GetFloat(field, def.x, 0),
		GetFloat(field, def.y, 1),
		GetFloat(field, def.z, 2));
}

bool Config::AddFloat4(const char * field, const float4 & value)
{
	return AddArrayFloat(field, &value.x, 4);
}

float4 Config::GetFloat4(const char * field, const float4 & def)
{
	return float4(
		GetFloat(field, def.x, 0),
		GetFloat(field, def.y, 1),
		GetFloat(field, def.z, 2),
		GetFloat(field, def.w, 3));
}

bool Config::AddFloat4x4(const char *field, const float4x4 &value)
{
    return AddArrayFloat(field, reinterpret_cast<const float*>(&value), 16);
}

float4x4 Config::GetFloat4x4(const char *field, const float4x4 &def/* = float4x4::identity*/)
{
    return float4x4(
        GetFloat(field, def.At(0, 0), 0), 
        GetFloat(field, def.At(0, 1), 1),
        GetFloat(field, def.At(0, 2), 2),
        GetFloat(field, def.At(0, 3), 3),
        GetFloat(field, def.At(1, 0), 4), 
        GetFloat(field, def.At(1, 1), 5),
        GetFloat(field, def.At(1, 2), 6),
        GetFloat(field, def.At(1, 3), 7),
        GetFloat(field, def.At(2, 0), 8), 
        GetFloat(field, def.At(2, 1), 9),
        GetFloat(field, def.At(2, 2), 10),
        GetFloat(field, def.At(2, 3), 11),
        GetFloat(field, def.At(3, 0), 12), 
        GetFloat(field, def.At(3, 1), 13),
        GetFloat(field, def.At(3, 2), 14),
        GetFloat(field, def.At(3, 3), 15));
}