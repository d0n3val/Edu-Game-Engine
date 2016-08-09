#ifndef __CONFIG_H__
#define __CONFIG_H__

// C++ wrapper for JSON parser library "Parson"

struct json_object_t;
typedef struct json_object_t JSON_Object;

struct json_value_t;
typedef struct json_value_t  JSON_Value;

class Config
{
public:
	Config(JSON_Object* section = nullptr);
	~Config();

	bool IsValid() const;
	bool CreateFromString(const char* string);
	void CreateEmpty();

	size_t Save(char** buf, const char* title_comment) const;

	int Size() const;
	Config GetSection(const char* section_name);
	Config AddSection(const char* section_name);
	Config AddNewArray();

	bool GetBool(const char * field, bool default, int index = -1) const;
	int GetInt(const char* field, int default, int index = -1) const;
	float GetFloat(const char* field, float default, int index = -1) const;
	const char* GetString(const char* field, const char* default, int index = -1) const;

	bool AddBool(const char* field, bool value);
	bool AddInt(const char* field, int value);
	bool AddFloat(const char* field, float value);
	bool AddString(const char* field, const char* string);
	Config AddArrayEntry();

private:
	JSON_Value* FindValue(const char* field, int index) const;

private:
	JSON_Value* vroot = nullptr;
	JSON_Object* root = nullptr;
	bool needs_removal = false;
};

#endif // __CONFIG_H__
