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
	bool CreateFromFile(const char* file_name);

	int Size() const;
	Config GetSection(const char* section_name);

	bool GetBool(const char * field, int index = -1) const;
	int GetInt(const char* field, int index = -1) const;
	float GetFloat(const char* field, int index = -1) const;
	const char* GetString(const char* field, int index = -1) const;

private:
	JSON_Value* vroot = nullptr;
	JSON_Object* root = nullptr;
	bool needs_removal = false;
};

#endif // __CONFIG_H__
