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

	void Save(Config& config) const override;
	void Load(const Config& config) override;

public:
	std::string name;
	double duration;
	double ticks_per_second;

	struct bone_transform
	{
		std::string bone_name;

		template<class TYPE>
		struct key
		{
			void Alloc(uint size)
			{
				count = size;
				time = new double[size];
				value = (TYPE*) _aligned_malloc_dbg(sizeof(TYPE) * size, 16, __FILE__, __LINE__);
			}

			~key()
			{
				RELEASE_ARRAY(time);
				if (value != nullptr)
					_aligned_free_dbg(value); // for quats!
			}

			uint count = 0;
			double* time = nullptr;
			TYPE* value = nullptr;
		};

		key<float3> positions;
		key<float3> scales;
		key<Quat> rotations;
	};

	uint num_keys = 0;
	bone_transform* bone_keys = nullptr;
};

#endif // __RESOURCE_ANIMATION_H__