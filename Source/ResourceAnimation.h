#ifndef __RESOURCE_ANIMATION_H__
#define __RESOURCE_ANIMATION_H__

#include "Resource.h"
#include "Math.h"
#include <vector>
#include <map>

#define ANIM_NAME_SIZE 25
#define BONE_NAME_SIZE 50 

class ResourceAnimation : public Resource
{
public:

	ResourceAnimation(UID id);
	virtual ~ResourceAnimation();

	bool LoadInMemory() override;
    void ReleaseFromMemory() override;

	void Save(Config& config) const override;
	void Load(const Config& config) override;

	float GetDurationInSecs() const;

	void FindBoneTransformation(float time, uint bone_index, float3& pos, Quat& rot, float3& scale, bool interpolate = true) const;

public:
	std::string name;
	double duration;
	double ticks_per_second;

	struct bone_transform
	{
		std::string bone_name;

		struct TRS
		{
			void Alloc(uint count, uint values_per_item = 3)
			{
				this->count = count;
				time = new double[count];
				value = new float[count * values_per_item];
			}

			~TRS()
			{
				RELEASE_ARRAY(time);
				RELEASE_ARRAY(value);
			}

			uint count = 0;
			double* time = nullptr;
			float* value = nullptr; // for rotations we should have count * 4 floats
		};

		TRS positions;
		TRS scales;
		TRS rotations;
	};

	uint num_keys = 0;
	bone_transform* bone_keys = nullptr;
};

#endif // __RESOURCE_ANIMATION_H__
