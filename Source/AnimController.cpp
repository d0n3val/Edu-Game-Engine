#include "Globals.h"

#include "AnimController.h"
#include "ResourceAnimation.h"
#include "Application.h"
#include "ModuleResources.h"

#include "mmgr/mmgr.h"

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

bool AnimController::GetTransform(const HashString& channel_name, float3& position, Quat& rotation) const
{
    if(current != nullptr)
    {
        return GetTransformInstance(current, channel_name, position, rotation);
    }

	return false;
}

bool AnimController::GetTransformInstance(Instance* instance, const HashString& channel_name, float3& position, Quat& rotation) const
{
	const ResourceAnimation* animation = static_cast<ResourceAnimation*>(App->resources->Get(instance->clip));

    if(animation != nullptr)
    {
        unsigned channel_index  = animation->FindChannelIndex(channel_name);

        if(channel_index < animation->GetNumChannels())
        {
            assert(instance->time <= animation->duration);

            float pos_key = float(instance->time*(animation->GetNumPositions(channel_index)-1))/float(animation->GetDuration());
            float rot_key = float(instance->time*(animation->GetNumRotations(channel_index)-1))/float(animation->GetDuration());

            unsigned pos_index = unsigned(pos_key);
            unsigned rot_index = unsigned(rot_key);

            float pos_lambda = pos_key-float(pos_index);
            float rot_lambda = rot_key-float(rot_index);

            if(pos_lambda > 0.0f)
            {
                position = Interpolate(animation->GetPosition(channel_index, pos_index), animation->GetPosition(channel_index, pos_index+1), pos_lambda);
            }
            else
            {
                position = animation->GetPosition(channel_index, pos_index);
            }

            if(rot_lambda > 0.0f)
            {
                rotation = Interpolate(animation->GetRotation(channel_index, rot_index), animation->GetRotation(channel_index, rot_index+1), rot_lambda);
            }
            else
            {
                rotation = animation->GetRotation(channel_index, rot_index);
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

float3 AnimController::Interpolate(const float3& first, const float3& second, float lambda) const
{
    return first*(1.0f-lambda)+second*lambda;
}

Quat AnimController::Interpolate(const Quat& first, const Quat& second, float lambda) const
{
    Quat result;

    // note: ensure minimal angle interpolation
	float dot = first.Dot(second); 

	if(dot >= 0.0f)
	{
		result.x = first.x*(1.0f-lambda)+second.x*lambda;
		result.y = first.y*(1.0f-lambda)+second.y*lambda;
		result.z = first.z*(1.0f-lambda)+second.z*lambda;
		result.w = first.w*(1.0f-lambda)+second.w*lambda;
	}
	else
	{
		result.x = first.x*(1.0f-lambda)-second.x*lambda;
		result.y = first.y*(1.0f-lambda)-second.y*lambda;
		result.z = first.z*(1.0f-lambda)-second.z*lambda;
		result.w = first.w*(1.0f-lambda)-second.w*lambda;
	}

	result.Normalize();

	return result;
}
