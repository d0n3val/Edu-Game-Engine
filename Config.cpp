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

bool Config::CreateFromFile(const char * file_name)
{
	bool ret = false;

	if (file_name != nullptr)
	{
		vroot = json_parse_file_with_comments(file_name);
		if (vroot != nullptr) {
			root = json_value_get_object(vroot);
			ret = needs_removal = true;
		}
	}

	return ret;
}


int Config::Size() const
{
	return (root) ? json_object_get_count(root) : 0;
}

Config Config::GetSection(const char * section_name)
{
	return Config(json_object_get_object(root, section_name));
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