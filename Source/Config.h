#ifndef __CONFIG_H__
#define __CONFIG_H__

// C++ wrapper for JSON parser library "Parson"
// http://www.w3schools.com/json/json_syntax.asp

struct json_object_t;
typedef struct json_object_t JSON_Object;

struct json_value_t;
typedef struct json_value_t  JSON_Value;

struct json_array_t;
typedef struct json_array_t  JSON_Array;

class Config
{
public:
	Config();
	Config(const char* data);
	Config(JSON_Object* section);
	~Config();

	bool IsValid() const;

	size_t Save(char** buf, const char* title_comment) const;

	int Size() const;
	Config GetSection(const char* section_name);
	Config AddSection(const char* section_name);

	bool GetBool(const char * field, bool default, int index = -1) const;
	int GetInt(const char* field, int default, int index = -1) const;
	float GetFloat(const char* field, float default, int index = -1) const;
	const char* GetString(const char* field, const char* default, int index = -1) const;
	int GetArrayCount(const char * field) const;
	Config GetArray(const char* field, int index) const;

	bool AddBool(const char* field, bool value);
	bool AddInt(const char* field, int value);
	bool AddFloat(const char* field, float value);
	bool AddString(const char* field, const char* string);
	bool AddArray(const char* array_name);
	bool AddArrayEntry(const Config& config);

	bool AddArrayBool(const char* field, const bool* values, int size);
	bool AddArrayInt(const char* field, const int* values, int size);
	bool AddArrayFloat(const char* field, const float* values, int size);
	bool AddArrayString(const char* field, const char** values, int size);

private:
	JSON_Value* FindValue(const char* field, int index) const;

private:
	JSON_Value* vroot = nullptr;
	JSON_Object* root = nullptr;
	JSON_Array* array = nullptr;
	bool needs_removal = false;
};

#endif // __CONFIG_H__
