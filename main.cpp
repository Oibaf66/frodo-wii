#include <SDL_image.h>
#include <SDL_ttf.h>

#include "gui.hh"
#include "utils.hh"
#include <C64.h>

SDL_Surface *screen;
C64 *TheC64;

static void run(void)
{
	while(1)
	{
		SDL_Event ev;

		while (SDL_PollEvent(&ev)) {
	        	if (ev.type == SDL_QUIT)
	                	exit(1);

	        	Gui::gui->pushEvent(&ev);
	        }
	        Gui::gui->runLogic();
	        Gui::gui->draw(screen);

	        SDL_Flip(screen);
		SDL_Delay(50);
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

int main(int argc, char *argv[])
{
	init();
	run();
	return 0;
}
