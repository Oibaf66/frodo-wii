#include <SDL_image.h>
#include <SDL_ttf.h>

#include "gui.hh"
#include "utils.hh"
#include <C64.h>

SDL_Surface *screen;
C64 *TheC64;

#define MS_PER_FRAME 28

static void run(void)
{
	Uint32 last_frame = SDL_GetTicks();

	while(1)
	{
		SDL_Event ev;
		Uint32 now = SDL_GetTicks();

		if (!Gui::gui->is_active)
			break;

		while (SDL_PollEvent(&ev)) {
	        	if (ev.type == SDL_QUIT)
	                	exit(1);

	        	Gui::gui->pushEvent(&ev);
	        }
	        Gui::gui->runLogic();
	        Gui::gui->draw(screen);

	        SDL_Flip(screen);

	        if ( (now - last_frame) < MS_PER_FRAME)
			SDL_Delay( MS_PER_FRAME - (now - last_frame));
	}
}

static void init(void)
{
	/* Creation of the mocks */
	TheC64 = new C64();

	screen = SDL_SetVideoMode(640, 480, 16,
			SDL_DOUBLEBUF);
	panic_if(!screen, "Cannot initialize video: %s\n", SDL_GetError());
	TTF_Init();

	Gui::init();
	Gui::gui->activate();
}

static void fini(void)
{
	delete Gui::gui;
}

int main(int argc, char *argv[])
{
	init();
	run();
	fini();

	SDL_Quit();

	return 0;
}
