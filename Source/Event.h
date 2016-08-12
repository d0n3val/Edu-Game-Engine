#ifndef __EVENT_H__
#define __EVENT_H__

struct Event
{
	enum EventType
	{
		gameobject_destroyed,
		window_resize,
		file_dropped,
		invalid
	} type;

	union
	{
		struct 
		{
			const char* ptr;
		} string;

		struct 
		{
			int x, y;
		} point2d;
	};

	Event(EventType type) : type(type)
	{}
};


#endif // __EVENT_H__