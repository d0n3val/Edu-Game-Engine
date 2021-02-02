#include "Globals.h"

#include "AnimController.h"
#include "ResourceAnimation.h"
#include "Application.h"
#include "ModuleResources.h"

#include "Leaks.h"

namespace
{

    inline float Interpolate(float first, float second, float lambda) 
    {
        return first * (1.0f - lambda) + second * lambda;
    }

    inline float3 Interpolate(const float3& first, const float3& second, float lambda) 
    {
        return float3::Lerp(first, second, lambda);
    }

    inline Quat Interpolate(const Quat& first, const Quat& second, float lambda) 
    {
        // note: ensure minimal angle interpolation
        if(first.Dot(second) >= 0.0f)
        {
            return Quat::Lerp(first, second, lambda).Normalized();
        }
        return Quat::Lerp(first, second.Neg(), lambda).Normalized();
    }

}

AnimController::AnimController()
{
}

AnimController::~AnimController()
{
    if(current != nullptr)
    {
        ReleaseInstance(current);
    }
}

void AnimController::Update(unsigned elapsed)
{
    if(current != nullptr)
    {
        UpdateInstance(current, elapsed);
    }
}

void AnimController::UpdateInstance(Instance* instance, unsigned elapsed)
{
	ResourceAnimation* anim = static_cast<ResourceAnimation*>(App->resources->Get(instance->clip));


	if (anim != nullptr && anim->GetDuration() > 0)
	{
        unsigned me_elapsed = unsigned(elapsed*instance->speed);
        me_elapsed = me_elapsed % anim->GetDuration();
		unsigned to_end = anim->GetDuration() - instance->time;

		if (me_elapsed <= to_end)
		{
			instance->time += me_elapsed;
		}
		else if (instance->loop)
		{
			instance->time = (me_elapsed - to_end);
		}
		else
		{
			instance->time = anim->GetDuration();
		}

		assert(instance->time <= anim->GetDuration());
	}

	if(instance->next != nullptr)
	{
		unsigned to_end = instance->fade_duration-instance->fade_time;
		if(elapsed <= to_end)
		{
			instance->fade_time += elapsed;
			UpdateInstance(instance->next, elapsed);
		}
		else
		{
			ReleaseInstance(instance->next);
			instance->next = nullptr;
			instance->fade_time = instance->fade_duration = 0;
		}
	}
}

void AnimController::Play(UID clip, bool loop, unsigned fade_time)
{
    Instance* new_instance = new Instance;
    new_instance->clip = clip;
    new_instance->loop = loop;
    new_instance->fade_duration = fade_time;
    new_instance->next = current;

    current = new_instance;
}

void AnimController::Stop()
{
    if(current != nullptr)
    {
        ReleaseInstance(current);
		current = nullptr;
    }
}

void AnimController::ReleaseInstance(Instance* instance)
{
	do
	{
		Instance* next = instance->next;
		delete instance;
		instance = next;
	} while (instance != nullptr);
}

bool AnimController::GetWeights(const HashString& morph_name, float* weights, uint num_weights) const
{
    if (current != nullptr)
    {
        return GetWeightsInstance(current, morph_name, weights, num_weights, 1.0f);
    }

    return false;
}

bool AnimController::GetWeightsInstance(Instance* instance, const HashString& morph_name, float*& weights, uint num_weights, float lambda) const
{
	const ResourceAnimation* animation = static_cast<ResourceAnimation*>(App->resources->Get(instance->clip));

    if(animation != nullptr)
    {
        unsigned channel_index  = animation->FindMorphIndex(morph_name);

        if(channel_index < animation->GetNumMorphChannels())
        {
            assert(instance->time <= animation->GetDuration());

            float key          = float(instance->time*(animation->GetNumKeys(channel_index)-1))/float(animation->GetDuration());
            unsigned key_index = unsigned(key);
            float key_lambda   = key-float(key_index);

            assert(num_weights == animation->GetNumWeights(channel_index));

            if(key_lambda > 0.0f)
            {
                for (uint i = 0; i < num_weights; ++i)
                {
                    float w0 = animation->GetWeight(channel_index, i, key_index);
                    float w1 = animation->GetWeight(channel_index, i, key_index+1);
                    weights[i] = Interpolate(weights[i], Interpolate(w0, w1, key_lambda), lambda);
                }
            }
            else
            {
                for(uint i=0; i< num_weights; ++i)
                {
                    weights[i] = Interpolate(weights[i], animation->GetWeight(channel_index, i, key_index), lambda);
                }
            }

            if(instance->next != nullptr)
            {
                assert(instance->fade_duration > 0.0f);

               float blend_lambda = float(instance->fade_time) / float(instance->fade_duration);

                GetWeightsInstance(instance->next, morph_name, weights, num_weights, blend_lambda*lambda);
            }

            return true;
        }
    }

	return false;
}

bool AnimController::GetTransform(const std::string& channel_name, float3& position, Quat& rotation) const
{
    if(current != nullptr)
    {
        return GetTransformInstance(current, channel_name, position, rotation);
    }

	return false;
}

bool AnimController::GetTransformInstance(Instance* instance, const std::string& channel_name, float3& position, Quat& rotation) const
{
	const ResourceAnimation* animation = static_cast<ResourceAnimation*>(App->resources->Get(instance->clip));

    if(animation != nullptr)
    {
        const ResourceAnimation::Channel* channel = animation->GetChannel(channel_name);

        if(channel != nullptr)
        {
            assert(instance->time <= animation->GetDuration());


            float pos_key = float(instance->time*(channel->num_positions-1))/float(animation->GetDuration());
            float rot_key = float(instance->time*(channel->num_rotations-1))/float(animation->GetDuration());

            unsigned pos_index = unsigned(pos_key);
            unsigned rot_index = unsigned(rot_key);

            float pos_lambda = pos_key-float(pos_index);
            float rot_lambda = rot_key-float(rot_index);

            if(pos_lambda > 0.0f)
            {
                position = Interpolate(channel->positions[pos_index], channel->positions[pos_index+1], pos_lambda);
            }
            else
            {
                position = channel->positions[pos_index];
            }

            if(rot_lambda > 0.0f)
            {
                rotation = Interpolate(channel->rotations[rot_index], channel->rotations[rot_index+1], rot_lambda);
            }
            else
            {
                rotation = channel->rotations[rot_index];
            }

            if(instance->next != nullptr)
            {
                assert(instance->fade_duration > 0.0f);

                float3 next_position;
                Quat next_rotation;

                if(GetTransformInstance(instance->next, channel_name, next_position, next_rotation))
                {
                    float blend_lambda = float(instance->fade_time) / float(instance->fade_duration);

                    position = Interpolate(next_position, position, blend_lambda);
                    rotation = Interpolate(next_rotation, rotation, blend_lambda);
                }
            }

            return true;
        }
    }

	return false;
}

