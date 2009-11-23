#include "menu.hh"
#include "utils.hh"

class PrintMenu : public Menu
{
public:
	PrintMenu(TTF_Font *font) : Menu(font)
	{
	}

	virtual void selectCallback(int which)
	{
		printf("entry %d selected: %s\n", which, this->pp_msgs[which]);
	}

	virtual void escapeCallback(int which)
	{
		printf("entry %d escaped: %s\n", which, this->pp_msgs[which]);
	}
};

SDL_Surface *screen;
PrintMenu *g_menu;

static void run(void)
{
	while(1)
	{
		SDL_Event ev;
	        while (SDL_PollEvent(&ev)) {
	        	if (ev.type == SDL_QUIT)
	                	exit(1); break;

	        	g_menu->pushEvent(&ev); break;
	        }
	        g_menu->draw(screen, 0, 80, 400, 400);

		SDL_Delay(50);
	}
}

static void init(void)
{
	TTF_Font *fnt;

	screen = SDL_SetVideoMode(640, 480, 16,
			SDL_DOUBLEBUF);
	panic_if(!screen, "Cannot initialize video: %s\n", SDL_GetError());
	TTF_Init();


	fnt = read_and_alloc_font("font.ttf");

	g_menu = new PrintMenu(fnt);
}

int main(int argc, char *argv[])
{
	init();
	run();
	return 0;
}
