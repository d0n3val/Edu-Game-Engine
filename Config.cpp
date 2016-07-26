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

bool Config::GetBool(const char * field, int index) const
{
	if (root == nullptr)
		return false;

	if(index < 0)
		return json_object_get_boolean(root, field) != 0;
	else
	{
		JSON_Array* array = json_object_get_array(root, field);
		return (array) ? (json_array_get_boolean(array, index) != 0) : false;
	}
}

int Config::GetInt(const char * field, int index) const
{
	if (root == nullptr)
		return 0;

	if(index < 0)
		return (int) json_object_get_number(root, field);
	else
	{
		JSON_Array* array = json_object_get_array(root, field);
		return (array) ? (int) json_array_get_number(array, index) : 0;
	}
}

float Config::GetFloat(const char * field, int index) const
{
	if (root == nullptr)
		return 0.f;

	if(index < 0)
		return (float) json_object_get_number(root, field);
	else
	{
		JSON_Array* array = json_object_get_array(root, field);
		return (array) ? (float) json_array_get_number(array, index) : 0.f;
	}
}

const char* Config::GetString(const char * field, int index) const
{
	if (root == nullptr)
		return nullptr;

	if(index < 0)
		return json_object_get_string(root, field);
	else
	{
		JSON_Array* array = json_object_get_array(root, field);
		return (array) ? json_array_get_string(array, index) : nullptr;
	}
}