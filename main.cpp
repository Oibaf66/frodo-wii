#include <SDL_image.h>
#include <SDL_ttf.h>

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
	                	exit(1);

	        	g_menu->pushEvent(&ev);
	        }
	        g_menu->draw(screen, 80, 80, 400, 400);

	        SDL_Flip(screen);
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
	SDL_Surface *bg_left, *bg_right, *bg_middle;

	screen = SDL_SetVideoMode(640, 480, 16,
			SDL_DOUBLEBUF);
	panic_if(!screen, "Cannot initialize video: %s\n", SDL_GetError());

	TTF_Init();


	fnt = read_and_alloc_font("font.ttf");

	bg_left = IMG_Load("bg_left.png");
	bg_right = IMG_Load("bg_right.png");
	bg_middle = IMG_Load("bg_middle.png");
	panic_if( !bg_left || !bg_right || !bg_middle,
			"bg loading failed\n");

	g_menu = new PrintMenu(fnt);
	g_menu->setText(main_menu_messages);
	g_menu->setSelectedBackground(bg_left, bg_middle, bg_right);
}

int main(int argc, char *argv[])
{
	init();
	run();
	return 0;
}
