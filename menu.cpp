/*********************************************************************
*
* Copyright (C) 2004,2008,  Simon Kagstrom
*
* Filename:      menu.c
* Author:        Simon Kagstrom <simon.kagstrom@gmail.com>
* Description:   Code for menus (originally for Mophun)
*
* $Id$
*
********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "menu.hh"
#include "menutexts.h"

typedef struct
{
	int n_entries;
	int index;
	int sel;
} submenu_t;


typedef struct
{
	char title[256];
	const char **pp_msgs;
	TTF_Font  *p_font;
	int        x1,y1;
	int        x2,y2;
	int        text_w;
	int        text_h;

	int        n_submenus;
	submenu_t *p_submenus;

	int        cur_sel; /* Main selection */
	int        start_entry_visible;
	int        n_entries;

	NetworkUpdatePeerInfo *peers;
	int n_peers;
} menu_t;

#define IS_SUBMENU(p_msg) ( (p_msg)[0] == '^' )
#define IS_TEXT(p_msg) ( (p_msg)[0] == '#' || (p_msg)[0] == ' ' )
#define IS_MARKER(p_msg) ( (p_msg)[0] == '@' )


int fh, fw;

static int cmpstringp(const void *p1, const void *p2)
{
	const char *p1_s = *(const char**)p1;
	const char *p2_s = *(const char**)p2;

	/* Put directories first */
	if (*p1_s == '[' && *p2_s != '[')
		return -1;
	if (*p1_s != '[' && *p2_s == '[')
		return 1;
	return strcmp(* (char * const *) p1, * (char * const *) p2);
}

/* Return true if name ends with ext (for filenames) */
static bool ext_matches(const char *name, const char *ext)
{
	int len = strlen(name);
	int ext_len = strlen(ext);

	if (len <= ext_len)
		return false;
	return (strcmp(name + len - ext_len, ext) == 0);
	
}

static bool ext_matches_list(const char *name, const char **exts)
{
	for (const char **p = exts; *p; p++)
	{
		if (ext_matches(name, *p))
			return true;
	}

	return false;
}

static const char **get_file_list(const char *base_dir, const char *exts[])
{
	DIR *d = opendir(base_dir);
	const char **file_list;
	int cur = 0;
	struct dirent *de;
	int cnt = 16;

	if (!d)
		return NULL;

	file_list = (const char**)malloc(cnt * sizeof(char*));
	file_list[cur++] = strdup("None"); 
	file_list[cur] = NULL;

	for (de = readdir(d);
	de;
	de = readdir(d))
	{
		char buf[255];
		struct stat st;

		snprintf(buf, 255, "%s/%s", base_dir, de->d_name);
		if (stat(buf, &st) < 0)
			continue;
		if (S_ISDIR(st.st_mode))
		{
			char *p;
			size_t len = strlen(de->d_name) + 4;

			p = (char*)malloc( len );
			snprintf(p, len, "[%s]", de->d_name);
			file_list[cur++] = p;
			file_list[cur] = NULL;
		}
		else if (ext_matches_list(de->d_name, exts))
		{
			char *p;

			p = strdup(de->d_name);
			file_list[cur++] = p;
			file_list[cur] = NULL;
		}

		if (cur > cnt - 2)
		{
			cnt = cnt + 32;
			file_list = (const char**)realloc(file_list, cnt * sizeof(char*));
			if (!file_list)
				return NULL;
		}
	}
	closedir(d);
        qsort(&file_list[1], cur-1, sizeof(const char *), cmpstringp);

        return file_list;
}


static submenu_t *find_submenu(menu_t *p_menu, int index)
{
	int i;

	for (i=0; i<p_menu->n_submenus; i++)
	{
		if (p_menu->p_submenus[i].index == index)
			return &p_menu->p_submenus[i];
	}

	return NULL;
}

void menu_print_font(SDL_Surface *screen, int r, int g, int b,
		int x, int y, const char *msg)
{
#define _MAX_STRING 64
	SDL_Surface *font_surf;
	SDL_Rect dst = {x, y,  0, 0};
	SDL_Color color = {r, g, b};
	char buf[255];
	unsigned int i;

	memset(buf, 0, sizeof(buf));
	strncpy(buf, msg, 254);
	if (buf[0] != '|' && buf[0] != '^' && buf[0] != '.'
		&& buf[0] != '-' && buf[0] != ' ' && !strstr(buf, "  \""))
	{
		if (strlen(buf)>_MAX_STRING)
		{
			buf[_MAX_STRING-3] = '.';
			buf[_MAX_STRING-2] = '.';
			buf[_MAX_STRING-1] = '.';
			buf[_MAX_STRING] = '\0';
		}
	}
	/* Fixup multi-menu option look */
	for (i = 0; i < strlen(buf) ; i++)
	{
		if (buf[i] == '^' || buf[i] == '|')
			buf[i] = ' ';
	}

	font_surf = TTF_RenderText_Blended(menu_font, buf,
			color);
	if (!font_surf)
	{
		fprintf(stderr, "%s\n", TTF_GetError());
		exit(1);
	}

	SDL_BlitSurface(font_surf, NULL, screen, &dst);
	SDL_FreeSurface(font_surf);
}


static void menu_draw(SDL_Surface *screen, menu_t *p_menu, int sel)
{
	int font_height = TTF_FontHeight(p_menu->p_font);
	int line_height = (font_height + font_height / 4);
	int x_start = p_menu->x1;
	int y_start = p_menu->y1 + line_height;
	SDL_Rect r;
	int entries_visible = (p_menu->y2 - p_menu->y1) / line_height - 2;

	int i, y;
	char pTemp[256];

	if ( p_menu->n_entries * line_height > p_menu->y2 )
		y_start = p_menu->y1 + line_height;

	if (p_menu->cur_sel - p_menu->start_entry_visible > entries_visible)
	{
		while (p_menu->cur_sel - p_menu->start_entry_visible > entries_visible)
		{
			p_menu->start_entry_visible ++;
			if (p_menu->start_entry_visible > p_menu->n_entries)
			{
				p_menu->start_entry_visible = 0;
				break;
			}
		}
	}
	else if ( p_menu->cur_sel < p_menu->start_entry_visible )
		p_menu->start_entry_visible = p_menu->cur_sel;

	if (strlen(p_menu->title))
	{
		r.x = p_menu->x1;
		r.y = p_menu->y1;
		r.w = p_menu->x2 - p_menu->x1;
		r.h = line_height-1;
		if (sel < 0)
			SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0x40, 0x00, 0x00));
		else
			SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0x00, 0x00, 0xff));
		menu_print_font(screen, 0,0,0, p_menu->x1, p_menu->y1, p_menu->title);
	}

	for (i = p_menu->start_entry_visible; i <= p_menu->start_entry_visible + entries_visible; i++)
	{
		const char *msg = p_menu->pp_msgs[i];

		if (i >= p_menu->n_entries)
			break;
		if (IS_MARKER(msg))
			p_menu->cur_sel = atoi(&msg[1]);
		else
		{
			y = (i - p_menu->start_entry_visible) * line_height;

			if (sel < 0)
				menu_print_font(screen, 0x40,0x40,0x40,
						x_start, y_start + y, msg);
			else if (p_menu->cur_sel == i) /* Selected - color */
				menu_print_font(screen, 0,255,0,
						x_start, y_start + y, msg);
			else if (IS_SUBMENU(msg))
			{
				if (p_menu->cur_sel == i-1)
					menu_print_font(screen, 0x80,0xff,0x80,
							x_start, y_start + y, msg);
				else
					menu_print_font(screen, 0x40,0x40,0x40,
							x_start, y_start + y, msg);
			}
			else if (msg[0] == '#')
			{
				switch (msg[1])
				{
				case '1':
					menu_print_font(screen, 0,0,255,
							x_start, y_start + y, msg+2);
					break;
				case '2':
					menu_print_font(screen, 0x80,0x80,0x80,
							x_start, y_start + y, msg+2);
					break;
				default:
					menu_print_font(screen, 0x40,0x40,0x40,
							x_start, y_start + y, msg);
					break;							
				}
			}
			else /* Otherwise white */
				menu_print_font(screen, 0x40,0x40,0x40,
						x_start, y_start + y, msg);
			if (IS_SUBMENU(msg))
			{
				submenu_t *p_submenu = find_submenu(p_menu, i);
				int n_pipe = 0;
				int n;

				for (n=0; msg[n] != '\0'; n++)
				{
					/* Underline the selected entry */
					if (msg[n] == '|')
					{
						int16_t n_chars;

						for (n_chars = 1; msg[n+n_chars] && msg[n+n_chars] != '|'; n_chars++);

						n_pipe++;
						if (p_submenu->sel == n_pipe-1)
						{
							int w;
							int h;

							if (TTF_SizeText(p_menu->p_font, "X", &w, &h) < 0)
							{
								fw = w;
								fh = h;
								fprintf(stderr, "%s\n", TTF_GetError());
								exit(1);
							}

							r = (SDL_Rect){ x_start + (n+1) * w-1, y_start + (i+ 1 - p_menu->start_entry_visible) * ((h + h/4)) -3, (n_chars - 1) * w, 2};
							if (p_menu->cur_sel == i-1)
								SDL_FillRect(screen, &r,
										SDL_MapRGB(screen->format, 0x0,0xff,0x80));
							else
								SDL_FillRect(screen, &r,
										SDL_MapRGB(screen->format, 0x40,0x40,0x40));
							break;
						}
					}
				}
			}
		}
	}
}

static int get_next_seq_y(menu_t *p_menu, int v, int dy)
{
	if (v + dy < 0)
		return p_menu->n_entries - 1;
	if (v + dy > p_menu->n_entries - 1)
		return 0;
	return v + dy;
}

static void select_next(menu_t *p_menu, int dx, int dy)
{
	int next;
	char buffer[256];

	p_menu->cur_sel = get_next_seq_y(p_menu, p_menu->cur_sel, dy);
	next = get_next_seq_y(p_menu, p_menu->cur_sel, dy + 1);

	if (p_menu->pp_msgs[p_menu->cur_sel][0] == ' ' ||
			p_menu->pp_msgs[p_menu->cur_sel][0] == '#' ||
			IS_SUBMENU(p_menu->pp_msgs[p_menu->cur_sel]) )
		select_next(p_menu, dx, dy);

	/* If the next is a submenu */
	if (dx != 0 && IS_SUBMENU(p_menu->pp_msgs[next]))
	{
		submenu_t *p_submenu = find_submenu(p_menu, next);

		p_submenu->sel = (p_submenu->sel + dx) < 0 ? p_submenu->n_entries - 1 :
		(p_submenu->sel + dx) % p_submenu->n_entries;
	}
	else if (dx == -1 && !strcmp(p_menu->pp_msgs[0], "[..]"))
		p_menu->cur_sel = 0;
}

static void select_one(menu_t *p_menu, int sel)
{
	if (sel >= p_menu->n_entries)
		sel = 0;
	p_menu->cur_sel = sel;
	if (p_menu->pp_msgs[p_menu->cur_sel][0] == ' ' ||
			p_menu->pp_msgs[p_menu->cur_sel][0] == '#' ||
			IS_SUBMENU(p_menu->pp_msgs[p_menu->cur_sel]))
		select_next(p_menu, 0, 1);
}

static int is_submenu_title(menu_t *p_menu, int n)
{
	if (n+1 >= p_menu->n_entries)
		return 0;
	else
		return IS_SUBMENU(p_menu->pp_msgs[n+1]);
}


static void menu_init(menu_t *p_menu, const char *title, TTF_Font *p_font, const char **pp_msgs,
		int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	int submenu;
	int i;
	int j;

	memset(p_menu, 0, sizeof(menu_t));

	p_menu->pp_msgs = pp_msgs;
	p_menu->p_font = p_font;
	p_menu->x1 = x1;
	p_menu->y1 = y1;
	p_menu->x2 = x2;
	p_menu->y2 = y2;

	p_menu->text_w = 0;
	p_menu->n_submenus = 0;
	p_menu->peers = NULL;
	p_menu->n_peers = 0;
	strcpy(p_menu->title, title);

	for (p_menu->n_entries = 0; p_menu->pp_msgs[p_menu->n_entries]; p_menu->n_entries++)
	{
		int text_w_font;

		/* Is this a submenu? */
		if (IS_SUBMENU(p_menu->pp_msgs[p_menu->n_entries]))
		{
			p_menu->n_submenus++;
			continue; /* Length of submenus is unimportant */
		}

		if (TTF_SizeText(p_font, p_menu->pp_msgs[p_menu->n_entries], &text_w_font, NULL) != 0)
		{
			fprintf(stderr, "%s\n", TTF_GetError());
			exit(1);
		}
		if (text_w_font > p_menu->text_w)
			p_menu->text_w = text_w_font;
	}
	if (p_menu->text_w > p_menu->x2 - p_menu->x1)
		p_menu->text_w = p_menu->x2 - p_menu->x1;

	if ( !(p_menu->p_submenus = (submenu_t *)malloc(sizeof(submenu_t) * p_menu->n_submenus)) )
	{
		perror("malloc failed!\n");
		exit(1);
	}

	j=0;
	submenu = 0;
	for (; j < p_menu->n_entries; j++)
	{
		if (IS_SUBMENU(p_menu->pp_msgs[j]))
		{
			int n;

			p_menu->p_submenus[submenu].index = j;
			p_menu->p_submenus[submenu].sel = 0;
			p_menu->p_submenus[submenu].n_entries = 0;
			for (n=0; p_menu->pp_msgs[j][n] != '\0'; n++)
			{
				if (p_menu->pp_msgs[j][n] == '|')
					p_menu->p_submenus[submenu].n_entries++;
			}
			submenu++;
		}
	}
	p_menu->text_h = p_menu->n_entries * (TTF_FontHeight(p_font) + TTF_FontHeight(p_font) / 4);
}

static void menu_fini(menu_t *p_menu)
{
	free(p_menu->p_submenus);
}

uint32_t menu_wait_key_press(void)
{
	SDL_Event ev;
	uint32_t keys = 0;

	while (1)
	{
		if (SDL_PollEvent(&ev))
		{
			switch(ev.type)
			{
			case SDL_KEYDOWN:
				switch (ev.key.keysym.sym)
				{
				case SDLK_UP:
					keys |= KEY_UP;
					break;
				case SDLK_DOWN:
					keys |= KEY_DOWN;
					break;
				case SDLK_LEFT:
					keys |= KEY_LEFT;
					break;
				case SDLK_RIGHT:
					keys |= KEY_RIGHT;
					break;
				case SDLK_PAGEDOWN:
					keys |= KEY_PAGEDOWN;
					break;
				case SDLK_PAGEUP:
					keys |= KEY_PAGEUP;
					break;
				case SDLK_RETURN:
				case SDLK_SPACE:
					keys |= KEY_SELECT;
					break;
				case SDLK_HOME:
				case SDLK_ESCAPE:
					keys |= KEY_ESCAPE;
					break;
				default:
					break;
				}
				break;
				case SDL_QUIT:
					exit(0);
					break;
				default:
					break;

			}
			break;
		}
		if (keys != 0)
			return keys;
		SDL_Delay(100);
	}
	return keys;
}


extern char curdir[256];

static int menu_select_internal(SDL_Surface *screen,
		menu_t *p_menu, int *p_submenus, int sel,
		void (*select_next_cb)(menu_t *p, void *data) = NULL,
		void *select_next_cb_data = NULL)
{
	int ret = -1;

	for (int i = 0; i < p_menu->n_submenus; i++)
		p_menu->p_submenus[i].sel = p_submenus[i];

	while(1)
	{
		SDL_Rect r = {p_menu->x1, p_menu->y1,
				p_menu->x2 - p_menu->x1, p_menu->y2 - p_menu->y1};
		uint32_t keys;
		int sel_last = p_menu->cur_sel;

		SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0x00, 0x80, 0x80));

		menu_draw(screen, p_menu, 0);
		SDL_Flip(screen);

		keys = menu_wait_key_press();

		if (keys & KEY_UP)
			select_next(p_menu, 0, -1);
		else if (keys & KEY_DOWN)
			select_next(p_menu, 0, 1);
		else if (keys & KEY_PAGEUP)
			select_next(p_menu, 0, -6);
		else if (keys & KEY_PAGEDOWN)
			select_next(p_menu, 0, 6);
		else if (keys & KEY_LEFT)
			select_next(p_menu, -1, 0);
		else if (keys & KEY_RIGHT)
			select_next(p_menu, 1, 0);
		else if (keys & KEY_ESCAPE)
			break;
		else if (keys & KEY_SELECT)
		{
			ret = p_menu->cur_sel;
			int i;

			for (i=0; i<p_menu->n_submenus; i++)
				p_submenus[i] = p_menu->p_submenus[i].sel;
			break;
		}
		/* Invoke the callback when an entry is selected */
		if (sel_last != p_menu->cur_sel &&
				select_next_cb != NULL)
			select_next_cb(p_menu, select_next_cb_data);
	}

	SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));
	return ret;
}

int menu_select_sized(const char *title, const char **msgs, int *submenus, int sel,
		int x, int y, int x2, int y2,
		void (*select_next_cb)(menu_t *p, void *data) = NULL,
		void *select_next_cb_data = NULL)

{
	menu_t menu;
	int out;
	bool info;

	if (!strcmp(title, "Folder") || !strcmp(title, "Single File") ||
			!strcmp(title, "C-64 Disc") || !strcmp(title, "C-64 Tape") || sel < 0)
		info = false;
	else
		info = true;

	menu_init(&menu, title, menu_font, msgs,
			x, y, x2, y2);

	if (sel >= 0)
		select_one(&menu, sel);
	out = menu_select_internal(real_screen, &menu, submenus, sel,
			select_next_cb, select_next_cb_data);

	menu_fini(&menu);

	return out;
}

int menu_select(const char *title, const char **msgs, int *submenus)
{
	return menu_select_sized(title, msgs, submenus, 0,
			32, 32, FULL_DISPLAY_X-32, FULL_DISPLAY_Y-64);
}

int menu_select(const char **msgs, int *submenus)
{
	return menu_select("", msgs, submenus);
}

int menu_select_peer(NetworkUpdatePeerInfo *peers, int n_peers)
{
	menu_t menu;
	int out;
	const char **msgs;

	msgs = (const char**)calloc(n_peers + 2, sizeof(const char*));

	msgs[0] = "None (wait for peer to connect)";
	printf("Got %d peers\n", n_peers);
	for (int i = 0; i < n_peers; i++)
		msgs[i + 1] = (const char*)peers[i].name;

	menu_init(&menu, "", menu_font, msgs,
			32, 32, FULL_DISPLAY_X-32, FULL_DISPLAY_Y-64);
	menu.peers = peers;
	menu.n_peers = n_peers;

	out = menu_select_internal(real_screen, &menu, NULL, 0,
			NULL, NULL);

	menu_fini(&menu);
	free(msgs);

	return out;
}

extern "C" const char **DirD64(const char *FileName);

static void d64_list_cb(menu_t *p, void *data)
{
	const char *dp = (const char*)data;
	const char *exts[] = {".d64", ".D64", NULL};
	const char *name = p->pp_msgs[p->cur_sel];
	SDL_Rect r = {FULL_DISPLAY_X / 2, 32,
			FULL_DISPLAY_X / 2 - 32, FULL_DISPLAY_Y - 64};

	SDL_FillRect(real_screen, &r, SDL_MapRGB(real_screen->format, 0x00, 0x90, 0x90));
	if (ext_matches_list(name, exts))
	{
		char buf[255];
		const char **dir;
		menu_t menu;

		snprintf(buf, 255, "%s/%s", dp, name);
		dir = DirD64(buf);
		if (!dir)
			return;

		menu_init(&menu, "D64 contents", menu_font, dir,
				FULL_DISPLAY_X / 2, 32,
				FULL_DISPLAY_X / 2 - 32, FULL_DISPLAY_Y - 64);
		menu_draw(real_screen, &menu, 0);
		menu_fini(&menu);

		/* Cleanup dir list */
	        for ( int i = 0; dir[i]; i++ )
	        	free((void*)dir[i]);
	        free(dir);
	}
}


static const char *menu_select_file_internal(const char *dir_path,
		int x, int y, int x2, int y2)
{
	const char **file_list = get_file_list(dir_path);
	char *sel;
	char *out;
	int opt;

	if (file_list == NULL)
		return NULL;

	opt = menu_select_sized("Select file", file_list, NULL, 0,
			x, y, x2, y2,
			d64_list_cb, (void*)dir_path);

	if (opt < 0)
		return NULL;
	sel = strdup(file_list[opt]);

	/* Cleanup everything - file_list is NULL-terminated */
        for ( int i = 0; file_list[i]; i++ )
        	free((void*)file_list[i]);
        free(file_list);

	if (!sel)
		return NULL;
        /* If this is a folder, enter it recursively */
        if (sel[0] == '[')
        {
        	char buf[255];
        	int len = strlen(sel);
        	int s;
        	const char *p;

        	/* Remove trailing ] */
        	sel[len-1] = '\0';
        	s = snprintf(buf, 128, "%s/%s", dir_path, sel + 1);

        	/* We don't need this anymore */
        	free((void*)sel);
        	/* Too deep recursion! */
        	if (s >= sizeof(buf))
        		return NULL;
        	return menu_select_file(buf);
        }

	out = (char*)malloc(strlen(dir_path) + strlen(sel) + 4);
	snprintf(out, strlen(dir_path) + strlen(sel) + 4,
			"%s/%s", dir_path, sel);

	free(sel);
        return out;
}

const char *menu_select_file(const char *dir_path)
{
	return menu_select_file_internal(dir_path,
			32, 32, FULL_DISPLAY_X/2, FULL_DISPLAY_Y - 32);
}

static TTF_Font *read_font(const char *path)
{
	TTF_Font *out;
	SDL_RWops *rw;
	Uint8 *data = (Uint8*)malloc(1 * 1024*1024);
	FILE *fp = fopen(path, "r");

	if (!data) {
		fprintf(stderr, "Malloc failed\n");
		exit(1);
	}
	if (!fp) {
		fprintf(stderr, "Could not open font\n");
		exit(1);
	}
	fread(data, 1, 1 * 1024 * 1024, fp);
	rw = SDL_RWFromMem(data, 1 * 1024 * 1024);
	if (!rw) 
	{
		fprintf(stderr, "Could not create RW: %s\n", SDL_GetError());
		exit(1);
	}
	out = TTF_OpenFontRW(rw, 1, 20);
	if (!out)
	{
		fprintf(stderr, "Unable to open font %s\n", path);
		exit(1);		
	}
	fclose(fp);

	return out;
}


void menu_init()
{
	Uint8 *data64 = (Uint8*)malloc(1 * 1024*1024);

	menu_font = read_font(FONT_PATH);
	menu_font64 = read_font(FONT64_PATH);
}
