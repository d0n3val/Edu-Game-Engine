#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "Globals.h"
#include "Math.h"

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

	Config GetSection(const char* section_name) const;
	Config AddSection(const char* section_name);

    bool HasBool(const char* field, int index = -1) const;
    bool HasInt(const char* field, int index = -1) const;
    bool HasUInt(const char* field, int index = -1) const;
    bool HasUID(const char* field, int index = -1) const;
    bool HasDouble(const char* field, int index = -1) const;
	bool HasFloat(const char* field, int index = -1) const;
	bool HasString(const char* field, int index = -1) const;

	bool GetBool(const char * field, bool default = false, int index = -1) const;
	int GetInt(const char* field, int default = 0, int index = -1) const;
	uint GetUInt(const char* field, uint default = 0, int index = -1) const;
	UID GetUID(const char* field, UID default = 0, int index = -1) const;
	double GetDouble(const char * field, double default = 0.0, int index = -1) const;
	float GetFloat(const char* field, float default = 0.f, int index = -1) const;
	const char* GetString(const char* field, const char* default = nullptr, int index = -1) const;

	int GetArrayCount(const char * field) const;
	Config GetArray(const char* field, int index) const;

	bool AddBool(const char* field, bool value);
	bool AddInt(const char* field, int value);
	bool AddUInt(const char* field, uint value);
	bool AddUID(const char* field, UID value);
	bool AddDouble(const char * field, double value);
	bool AddFloat(const char* field, float value);
	bool AddString(const char* field, const char* string);
	bool AddArray(const char* array_name);
	bool AddArrayEntry(const Config& config);

	bool AddArrayBool(const char* field, const bool* values, int size);
	bool AddArrayInt(const char* field, const int* values, int size);
	bool AddArrayUInt(const char* field, const uint* values, int size);
	bool AddArrayUID(const char* field, const UID* values, int size);
	bool AddArrayFloat(const char* field, const float* values, int size);
	bool AddArrayFloat3(const char* field, const float3* values, int size);
	bool AddArrayString(const char* field, const char** values, int size);

	// Custom
	bool    AddFloat2(const char* field, const float2& value);
	float2  GetFloat2(const char* filed, const float2& default = float2::zero);
	bool    AddFloat3(const char* field, const float3& value);
	float3  GetFloat3(const char* field, const float3& default = float3::zero);
	bool    AddFloat4(const char* field, const float4& value);
	float4  GetFloat4(const char* field, const float4& default = float4::zero);

	JSON_Object* GetRoot() { return root; }

private:
	JSON_Value* FindValue(const char* field, int index) const;

private:
	JSON_Value* vroot = nullptr;
	JSON_Object* root = nullptr;
	JSON_Array* array = nullptr;
	bool needs_removal = false;
};

#endif // __CONFIG_H__
