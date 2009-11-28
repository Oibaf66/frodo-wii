#include <SDL_image.h>
#include <SDL_ttf.h>

#include "frodo_menu.hh"
#include "utils.hh"

SDL_Surface *screen;
static Gui *g_gui;

static void run(void)
{
	while(1)
	{
		SDL_Event ev;

		while (SDL_PollEvent(&ev)) {
	        	if (ev.type == SDL_QUIT)
	                	exit(1);

	        	g_gui->pushEvent(&ev);
	        }
	        g_gui->runLogic();
	        g_gui->draw(screen);

	        SDL_Flip(screen);
		SDL_Delay(50);
	}
}

static void init(void)
{
	screen = SDL_SetVideoMode(640, 480, 16,
			SDL_DOUBLEBUF);
	panic_if(!screen, "Cannot initialize video: %s\n", SDL_GetError());
	TTF_Init();

	g_gui = new Gui();
	panic_if(!g_gui->setTheme("themes/default"),
			"Setting theme failed\n");
	g_gui->activate();
}

int main(int argc, char *argv[])
{
	init();
	run();
	return 0;
}
