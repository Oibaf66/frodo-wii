#include <SDL_image.h>
#include <SDL_ttf.h>

#include "menu.hh"
#include "sdl_ttf_font.hh"
#include "utils.hh"

class PrintMenu : public Menu
{
public:
	PrintMenu(Font *font) : Menu(font)
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
SDL_Surface *g_background;
PrintMenu *g_menu;

static void run(void)
{
	while(1)
	{
		SDL_Event ev;

		while (SDL_PollEvent(&ev)) {
	        	if (ev.type == SDL_QUIT)
	                	exit(1);

	        	g_menu->pushEvent(&ev);
	        }
	        g_menu->runLogic();
	        g_menu->draw(screen, 80, 80, 400, 400);

	        SDL_Flip(screen);
	        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0));
	        SDL_BlitSurface(g_background, NULL, screen, NULL);
		SDL_Delay(50);
	}
}

const char *main_menu_messages[] = {
                /*02*/          "File",
                /*03*/          "^|Insert|Start",
                /*04*/          "States",
                /*05*/          "^|Load|Save|Delete",
                /*06*/          "Keyboard",
                /*07*/          "^|Type|Macro|Bind",
                /*08*/          " ",
                /*09*/          "Reset the C=64",
                /*10*/          "Networking",
                /*11*/          "Options",
                /*12*/          "Advanced Options",
                /*13*/          "Help",
                /*15*/          "Quit",
                NULL
};


static void init(void)
{
	TTF_Font *fnt;
	SDL_Surface *bg_left, *bg_right, *bg_middle,
		*bg_submenu_left, *bg_submenu_right, *bg_submenu_middle;

	screen = SDL_SetVideoMode(640, 480, 16,
			SDL_DOUBLEBUF);
	panic_if(!screen, "Cannot initialize video: %s\n", SDL_GetError());

	TTF_Init();


	fnt = read_and_alloc_font("font.ttf", 18);

	g_background = IMG_Load("themes/default/background.png");

	bg_left = IMG_Load("themes/default/bg_left.png");
	bg_right = IMG_Load("themes/default/bg_right.png");
	bg_middle = IMG_Load("themes/default/bg_middle.png");
	bg_submenu_left = IMG_Load("themes/default/bg_submenu_left.png");
	bg_submenu_right = IMG_Load("themes/default/bg_submenu_right.png");
	bg_submenu_middle = IMG_Load("themes/default/bg_submenu_middle.png");
	panic_if( !bg_left || !bg_right || !bg_middle ||
			!bg_submenu_left || !bg_submenu_right || !bg_submenu_middle,
			"bg loading failed\n");

	g_menu = new PrintMenu(new Font_TTF(fnt, 255, 255, 255));
	g_menu->setText(main_menu_messages);
	g_menu->setSelectedBackground(bg_left, bg_middle, bg_right,
			bg_submenu_left, bg_submenu_middle, bg_submenu_right);
}

int main(int argc, char *argv[])
{
	init();
	run();
	return 0;
}
