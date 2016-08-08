#include "Globals.h"
#include <Windows.h>
#include <stdlib.h>
#include "Application.h"
#include "MemLeaks.h"

// We need to include this here beacuse SDL overwrites main()
#include "SDL/include/SDL.h"
#pragma comment( lib, "SDL/libx86/SDL2.lib" )
#pragma comment( lib, "SDL/libx86/SDL2main.lib" )

enum main_states
{
	MAIN_CREATION,
	MAIN_START,
	MAIN_UPDATE,
	MAIN_FINISH,
	MAIN_EXIT
};

Application* App = nullptr;

int main(int argc, char ** argv)
{
	ReportMemoryLeaks();

	SDL_version version;
	SDL_GetVersion(&version);
	LOG("Starting EDU Engine from [%s]", argv[0]);
	LOG("Using SDL v%i.%i.%i", version.major, version.minor, version.patch);
	LOG("Machine with %ikb RAM and %i CPUs and %i Kb L1 cache", 
		SDL_GetSystemRAM(), SDL_GetCPUCount(), SDL_GetCPUCacheLineSize());
	LOG("CPU capabilities: %s%s%s%s%s%s%s%s%s%s%s",
		SDL_HasRDTSC() ? "RDTSC," : "",
		SDL_HasAltiVec() ? "AltiVec," : "",
		SDL_HasMMX() ? "MMX," : "",
		SDL_Has3DNow() ? "3DNow," : "",
		SDL_HasSSE() ? "SSE," : "",
		SDL_HasSSE2() ? "SSE2," : "",
		SDL_HasSSE3() ? "SSE3," : "",
		SDL_HasSSE41() ? "SSE41," : "",
		SDL_HasSSE42() ? "SSE42," : "",
		SDL_HasAVX() ? "AVX," : "",
		SDL_HasAVX2() ? "AVX2" : "" );

	int main_return = EXIT_FAILURE;
	main_states state = MAIN_CREATION;

	while (state != MAIN_EXIT)
	{
		switch (state)
		{
			case MAIN_CREATION:
			{

				LOG("-------------- Application Creation --------------");
				App = new Application();
				state = MAIN_START;
			} break;

			case MAIN_START:
			{
				LOG("-------------- Application Init --------------");
				if (App->Init() == false)
				{
					LOG("Application Init exits with ERROR");
					state = MAIN_EXIT;
				}
				else
				{
					state = MAIN_UPDATE;
					LOG("-------------- Application Update --------------");
				}

			} break;

			case MAIN_UPDATE:
			{
				int update_return = App->Update();

				if (update_return == UPDATE_ERROR)
				{
					LOG("Application Update exits with ERROR");
					state = MAIN_EXIT;
				}

				if (update_return == UPDATE_STOP)
					state = MAIN_FINISH;
			} break;

			case MAIN_FINISH:
			{
				LOG("-------------- Application CleanUp --------------");
				if (App->CleanUp() == false)
				{
					LOG("Application CleanUp exits with ERROR");
				}
				else
					main_return = EXIT_SUCCESS;

				state = MAIN_EXIT;

			} break;
		}
	}

	LOG("Exiting engine ...\n");
	RELEASE(App);
	return main_return;
}