#include "Config.h"
#include "parson.h"

// C++ wrapper for JSON parser library "Parson"

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

bool Config::CreateFromString(const char * string)
{
	bool ret = false;

	if (string != nullptr)
	{
		vroot = json_parse_string_with_comments(string);
		if (vroot != nullptr) {
			root = json_value_get_object(vroot);
			ret = needs_removal = true;
		}
	}

	return ret;
}

void Config::CreateEmpty()
{
	if (needs_removal == true)
		json_value_free(vroot);

	vroot = json_value_init_object();
	if (vroot != nullptr) 
		root = json_value_get_object(vroot);
	needs_removal = true;
}

size_t Config::Save(char** buf) const
{
	size_t written = json_serialization_size_pretty(vroot);
	*buf = new char[written];
	json_serialize_to_buffer_pretty(vroot, *buf, written);
	return written;
}

int Config::Size() const
{
	return (root) ? json_object_get_count(root) : 0;
}

Config Config::GetSection(const char * section_name)
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

bool Config::GetBool(const char * field, bool default, int index) const
{
	JSON_Value* value = FindValue(field, index);

	if (value && json_value_get_type(value) == JSONBoolean)
		return json_value_get_boolean(value) != 0;

	return default;
}

int Config::GetInt(const char * field, int default, int index) const
{
	JSON_Value* value = FindValue(field, index);

	if (value && json_value_get_type(value) == JSONNumber)
		return (int) json_value_get_number(value);

	return default;
}

float Config::GetFloat(const char * field, float default, int index) const
{
	JSON_Value* value = FindValue(field, index);

	if (value && json_value_get_type(value) == JSONNumber)
		return (float) json_value_get_number(value);

	return default;
}

const char* Config::GetString(const char * field, const char* default, int index) const
{
	JSON_Value* value = FindValue(field, index);

	if (value && json_value_get_type(value) == JSONString)
		return json_value_get_string(value);

	return default;
}

bool Config::AddBool(const char * field, bool value)
{
	return json_object_set_boolean(root, field, (value) ? 1 : 0) == JSONSuccess;
}

bool Config::AddInt(const char * field, int value)
{
	return json_object_set_number(root, field, (double) value) == JSONSuccess;
}

bool Config::AddFloat(const char * field, float value)
{
	return json_object_set_number(root, field, (float) value) == JSONSuccess;
}

bool Config::AddString(const char * field, const char * string)
{
	return json_object_set_string(root, field, string) == JSONSuccess;
}
