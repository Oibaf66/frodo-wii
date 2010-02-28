/*********************************************************************
 *
 * Copyright (C) 2009,  Simon Kagstrom
 *
 * Filename:      VirtualKeyboard.c
 * Author:        Simon Kagstrom <simon.kagstrom@gmail.com>
 * Description:   A virtual keyboard
 *
 * $Id$
 *
 ********************************************************************/
#include <SDL.h>

#include <utils.hh>

#include "virtual_keyboard.hh"
#include "gui.hh"

typedef struct virtkey
{
	const char *name;
	int kc;
	int flags;
} virtkey_t;

#define F_IS_SHIFT 1
#define F_IS_DONE  2
#define F_SHIFT_ON 4

#define INVALID_VIRTKEY ((struct virtkey){NULL, -1, 0})

static inline bool IS_INVALID_VIRTKEY(virtkey_t *k)
{
	return k->name == NULL && k->kc == -1 && k->flags == 0;
}
/*
  C64 keyboard matrix:

                                                                       Bit 7   6   5   4   3   2   1   0
                                                                     0    CUD  F5  F3  F1  F7 CLR RET DEL
                                                                     1    SHL  E   S   Z   4   A   W   3
                                                                     2     X   T   F   C   6   D   R   5
                                                                     3     V   U   H   B   8   G   Y   7
                                                                     4     N   O   K   M   0   J   I   9
                                                                     5     ,   @   :   .   -   L   P   +
                                                                     6     /   ^   =  SHR HOM  ;   *   �
                                                                     7    R/S  Q   C= SPC  2  CTL  <-  1
*/
#define MATRIX(a,b) (((a) << 3) | (b))

#define K(name, a,b) \
	{ name, MATRIX(a,b), 0 }
#define S(name, a,b) \
	{ name, MATRIX(a,b), F_IS_SHIFT }
#define N(name) \
	{ name, -1, 0 }
#define D(name) \
	{ name, -1, F_IS_DONE}
#define J(name, v) \
	{ name, 0x40 | (v), 0}

#define KEY_COLS 15
#define KEY_ROWS 8

static virtkey_t keys[KEY_COLS * KEY_ROWS] = {
	K("<-",7,1),       K("1", 7,0), K("2", 7,3), K("3", 1,0), K("4", 1,3), K("5", 2,0), K("6", 2,3), K("7", 3,0), K("8", 3,3), K("9", 4,0), K("0", 4,3), K("+", 5,0), K("-", 5,3), K("£", 6,0), K("Hom", 6,3),
	K("Cr", 7,2),      K("Q", 7,6), K("W", 1,1), K("E", 1,6), K("R", 2,1), K("T", 2,6), K("Y", 3,1), K("U", 3,6), K("I", 4,1), K("O", 4,6), K("P", 5,1), K("@", 5,6), K("*", 6,1), K("Au", 6,6),K("Rstr",4,0),
	K("R/Stp", 7,7),   K(NULL,0,0), K("A", 1,2), K("S", 1,5), K("D", 2,2), K("F", 2,5), K("G", 3,2), K("H", 3,5), K("J", 4,2), K("K", 4,5), K("L", 5,2), K(":", 5,5), K(";", 6,2), K("=", 6,5), K("Ret", 0,1),
	K("C=", 7,5),      S("Shft",1,7),K(NULL,0,0),K("Z", 1,4), K("X", 2,7), K("C", 2,4), K("V", 3,7), K("B", 3,4), K("N", 4,7), K("M", 4,4), K(",", 5,7), K(".", 5,4), K("/", 6,7), K("Dwn",0,7),K("Rgt", 0,2),
	N("None"),         K(NULL,0,0), K(NULL,0,0), K("space", 7,4),K(0, 0,0),K(NULL,0,0), K("f1",0,4), K("f3",0,5), K("f5",0,6), K("f7",0,3), K("Del",0,0),K(NULL,0,0), K(NULL,0,0), K(NULL,0,0), D("DONE"),
	K(NULL,0,0),       K(NULL,0,0), K(NULL,0,0), K(NULL,0,0), K(0, 0,0),K(0, 0,0),J("Joy up",1),K(NULL,0,0), K(NULL,0,0), K(NULL,0,0), K(NULL, 0,0),K(NULL,0,0), K(NULL,0,0), K(NULL,0,0), K(NULL, 0,0),
	K(0, 0,0), K(NULL,0,0), K(NULL,0,0), J("Joy left",4), K(0,0,0),K(0,0,0),J("Joy fire",0x10),K(0,0,0), K(NULL,0,0),J("Joy right",8),K(0,0,0),K(0, 0,0),K(0, 0,0), K(NULL,0,0), K(NULL,0,0),
	K(NULL,0,0),       K(0, 0,0),   K(NULL,0,0), K(NULL,0,0), K(0,0,0),K(0,0,0),J("Joy down",2),K(NULL,0,0),K(NULL,0,0), K(NULL,0,0), K(NULL,0,0), K(NULL,0,0), K(NULL, 0,0),K(NULL,0,0), K(NULL, 0,0),
};

static const char *shifted_names[KEY_COLS * KEY_ROWS] = {
	NULL,              "!",         "\"",        "#",         "$",         "%",         "&",         "'",         "(",         ")",         NULL,        NULL,        NULL,        NULL,        "Clr",
	NULL,              "q",         "w",         "e",         "r",         "t",         "y",         "u",         "i",         "o",         "p",        NULL,        NULL,        NULL,        NULL,
	NULL,              NULL,        "a",         "s",         "d",         "f",         "g",         "h",         "j",         "k",         "l",        "[",         "]",         NULL,        NULL,
	NULL,              NULL,        NULL,        "z",         "x",         "c",         "v",         "b",         "n",         "m",         "<",         ">",        "?",         "Up",        "Lft",
	NULL,              NULL,        NULL,        NULL,        NULL,        NULL,        "f2",        "f4",        "f6",        "f8",        "Ins",       NULL,        NULL,        NULL,        NULL,
};

VirtualKeyboard::VirtualKeyboard(Font *font) : GuiView(), ListenerManager()
{
	this->font = font;
	this->sel_x = 0;
	this->sel_y = 0;
	this->shift_on = false;
	this->kbd_only_input = false;
	this->default_shifted = false;

	this->is_active = false;
	this->buf_head = 0;
	this->buf_len = 255;
	this->buf = (struct virtkey*)xmalloc(sizeof(struct virtkey) * this->buf_len);
	memset(this->buf, 0, sizeof(struct virtkey) * this->buf_len);
}

VirtualKeyboard::~VirtualKeyboard()
{
	free(this->buf);
}

void VirtualKeyboard::draw(SDL_Surface *where, int x_base, int y_base, int w, int h)
{
	int key_w = w / KEY_COLS;
	int key_h = h / KEY_ROWS;

	for (int y = 0; y < KEY_ROWS; y++ )
	{
		for (int x = 0; x < KEY_COLS; x++ )
		{
			int which = y * KEY_COLS + x;
			virtkey_t key = keys[which];
			const char *what = key.name;

			/* Skip empty positions */
			if (key.name == NULL)
				continue;
			if (this->shift_on && shifted_names[which])
				what = shifted_names[which];

			if (this->sel_x == x && this->sel_y == y)
				highlight_background(where, Gui::gui->small_font,
						Gui::gui->bg_left, Gui::gui->bg_middle, Gui::gui->bg_right,
						x * key_w + x_base, y * key_h + y_base,
						this->font->getWidth(what), h);
			this->font->draw(where, what,
					x * key_w + x_base, y * key_h + y_base, w, h);
		}
	}
}

void VirtualKeyboard::selectNext(int dx, int dy)
{
	int next_x = (this->sel_x + dx) % KEY_COLS;
	int next_y = (this->sel_y + dy) % KEY_ROWS;
	virtkey_t key;

	if (next_x < 0)
		next_x = KEY_COLS + next_x;
	if (next_y < 0)
		next_y = KEY_ROWS + next_y;
	this->sel_x = next_x;
	this->sel_y = next_y;

	key = keys[ next_y * KEY_COLS + next_x ];

	/* Skip the empty spots */
	if (key.name == NULL)
	{
		if (dy != 0) /* Look left */
			this->selectNext(-1, 0);
		else
			this->selectNext(dx, dy);
	}
}

void VirtualKeyboard::toggleShift()
{
	this->shift_on = !this->shift_on;
}

const char *VirtualKeyboard::keycodeToString(int kc)
{
	bool shifted = kc & 0x80;
	int kc_raw = kc & ~0x80;
	const char *out = "Unknown";

	if (kc < 0)
		return "None";

	/* Just loop through all of them */
	for (int i = 0; i < KEY_COLS * KEY_ROWS; i++)
	{
		virtkey_t key = keys[i];

		if (key.kc == kc_raw && key.name != NULL)
		{
			out = key.name;

			if (shifted && shifted_names[i])
				out = shifted_names[i];
			break;
		}
	}

	return out;
}

int VirtualKeyboard::charToKeycode(char c)
{
	for (int i = 0; i < KEY_COLS * KEY_ROWS; i++)
	{
		virtkey_t key = keys[i];

		if (key.name != NULL)
		{
			if (strlen(key.name) == 1)
			{
				if (key.name[0] == c)
					return key.kc;
				if (shifted_names[i] && strlen(shifted_names[i]) == 1 &&
						shifted_names[i][0] == c)
					return key.kc | 0x80;
			}

			/* OK, ugly special cases, but these are pretty important */
			if (c == ' ' && strcmp(key.name, "space") == 0)
				return key.kc;
			if (c == '\n' && strcmp(key.name, "Ret") == 0)
				return key.kc;
		}
	}

	return -1;
}

virtkey_t VirtualKeyboard::eventToVirtkey(event_t ev)
{
	char c = (char)ev;

	for (int i = 0; i < KEY_COLS * KEY_ROWS; i++)
	{
		virtkey_t key = keys[i];

		if (key.name != NULL)
		{
			if (strlen(key.name) == 1)
			{
				if (key.name[0] == c)
					return key;
				if (shifted_names[i] && strlen(shifted_names[i]) == 1 &&
						shifted_names[i][0] == c)
					return key;
			}

			/* OK, ugly special cases, but these are pretty important */
			if ( (c == ' ' && strcmp(key.name, "space") == 0) ||
					(c == '\n' && strcmp(key.name, "Ret") == 0) )
				return key;
		}
	}

	return INVALID_VIRTKEY;
}

int VirtualKeyboard::stringToKeycode(const char *str)
{
	for (int i = 0; i < KEY_COLS * KEY_ROWS; i++)
	{
		virtkey_t key = keys[i];

		if (key.name != NULL)
		{
			if (strcmp(key.name, str) == 0)
				return key.kc;
		}
	}

	return 0;
}

const char VirtualKeyboard::keycodeToChar(int kc)
{
	const char *s = this->keycodeToString(kc);

	if (strcmp(s, "space") == 0)
		return ' ';
	if (strcmp(s, "Ret") == 0)
		return '\n';
	if (strcmp(s, "Del") == 0)
		return '\b';

	/* NULL is never, ever returned */
	return s[0];
}

void VirtualKeyboard::activate(bool default_shifted)
{
	this->default_shifted = default_shifted;
	this->shift_on = this->default_shifted;

	Gui::gui->kbd = this;
	this->is_active = true;
	memset(this->buf, 0, sizeof(struct virtkey) * this->buf_len);
	this->buf_head = 0;

	this->kbd_only_input = false;
}


void VirtualKeyboard::runLogic()
{
	event_t ev;
	virtkey_t ev_key;

	if (!this->is_active)
		return;

	ev = this->popEvent();
	if (ev == EVENT_NONE)
		return;

	/* Something was typed on the keyboard */
	ev_key = this->eventToVirtkey(ev);
	if ( !IS_INVALID_VIRTKEY(&ev_key) ) {
		this->pushKey(&ev_key);
	}

	if (ev & KEY_UP)
		this->selectNext(0, -1);
	else if (ev & KEY_DOWN)
		this->selectNext(0, 1);
	else if (ev & KEY_LEFT)
		this->selectNext(-1, 0);
	else if (ev & KEY_RIGHT)
		this->selectNext(1, 0);
	else if (ev & KEY_ESCAPE)
		this->deactivate();
	else if (ev & KEY_SELECT)
	{
		virtkey_t *key = &keys[ this->sel_y * KEY_COLS + this->sel_x ];

		if (!key)
			return;

		if (key->flags & F_IS_SHIFT)
			this->toggleShift();
		else if (key->flags & F_IS_DONE) /* We're done! */
			this->done();
		else if (strcmp(key->name, "Del") == 0)
		{
			if (this->buf_head > 1)
			{
				this->buf[this->buf_head - 1] = INVALID_VIRTKEY;
				this->buf_head--;
			}
		}
		else
			this->pushKey(key);
	}
}

void VirtualKeyboard::pushKey(struct virtkey *vk)
{
	int n_listeners = sizeof(this->listeners) / sizeof(*this->listeners);
	struct virtkey key = *vk;

	if (this->shift_on)
		key.flags |= F_SHIFT_ON;

	/* Add to buf */
	this->buf[this->buf_head] = key;

	this->buf_head++;
	if (this->buf_head >= this->buf_len - 1)
		this->buf_head = 0; /* OK, not good, but well... */
	for (int i = 0; i < n_listeners; i++)
	{
		if (this->listeners[i])
			((KeyboardListener*)this->listeners[i])->keyCallback(this->shift_on,
					vk->name);
	}
}

void VirtualKeyboard::deactivate()
{
	this->is_active = false;
	this->flushListeners();
	Gui::gui->kbd = NULL;
}

const char *VirtualKeyboard::getString()
{
	char *out = (char *)xmalloc(this->buf_head + 1);

	for (unsigned i = 0; i < this->buf_head; i++)
	{
		virtkey_t *k = &this->buf[i];
		int kc = k->kc | ((k->flags & F_SHIFT_ON) ? 0x80 : 0);

		out[i] = this->keycodeToChar(kc);
	}

	return out;
}

void VirtualKeyboard::done()
{
	int n_listeners = sizeof(this->listeners) / sizeof(*this->listeners);
	const char *str = this->getString();

	for (int i = 0; i < n_listeners; i++)
	{
		if (this->listeners[i])
			((KeyboardListener*)this->listeners[i])->stringCallback(str);
	}
	free((void*)str);
	this->deactivate();
}

void VirtualKeyboard::draw(SDL_Surface *where)
{
	SDL_Rect r = (SDL_Rect){10, 210, 0, 0};

	SDL_BlitSurface(Gui::gui->keyboard, NULL, where, &r);
	this->draw(where, 20, 240, 600, 240);

	const char *str = this->getString();
	if (strlen(str) > 0)
		this->font->draw(where, str,
				15, 212, 600, 240);
	free((void*)str);
}

void VirtualKeyboard::updateTheme()
{
	this->setFont(Gui::gui->small_font);
}

void VirtualKeyboard::pushEvent(event_t ev)
{
	Widget::pushEvent(ev);
}

void VirtualKeyboard::pushEvent(SDL_Event *ev)
{
	switch(ev->type)
	{
	case SDL_KEYDOWN:
		switch (ev->key.keysym.sym)
		{
		case SDLK_RSHIFT:
		case SDLK_LSHIFT:
			this->shift_on = !this->default_shifted; break;
		case SDLK_BACKSPACE:
			if (this->buf_head > 0)
				this->buf_head--;
			break;
		/* Handle via the standard widget implementation (except for space) */
		case SDLK_UP:
			this->kbd_only_input = false; Widget::pushEvent(KEY_UP); return;
		case SDLK_DOWN:
			this->kbd_only_input = false; Widget::pushEvent(KEY_DOWN); return;
		case SDLK_LEFT:
			this->kbd_only_input = false; Widget::pushEvent(KEY_LEFT); return;
		case SDLK_RIGHT:
			this->kbd_only_input = false; Widget::pushEvent(KEY_RIGHT); return;
		case SDLK_PAGEDOWN:
			this->kbd_only_input = false; Widget::pushEvent(KEY_PAGEDOWN); return;
		case SDLK_PAGEUP:
			this->kbd_only_input = false; Widget::pushEvent(KEY_PAGEUP); return;
		case SDLK_ESCAPE:
			this->kbd_only_input = false; Widget::pushEvent(KEY_ESCAPE); return;
		case SDLK_RETURN:
			if (this->kbd_only_input)
				this->done();
			else
				Widget::pushEvent(KEY_SELECT);
			return;
		case SDLK_SPACE ... SDLK_z:
			Widget::pushEvent((event_t)(ev->key.keysym.sym));
			this->kbd_only_input = true;
		default:
			break;
		} break;
	case SDL_KEYUP:
		switch (ev->key.keysym.sym)
		{
		case SDLK_RSHIFT:
		case SDLK_LSHIFT:
			this->shift_on = this->default_shifted; break;
		default: break;
		}
	default: break;
	}
}

/* The singleton */
VirtualKeyboard *VirtualKeyboard::kbd;


KeyboardListener::~KeyboardListener()
{
	VirtualKeyboard::kbd->unregisterListener(this);
}
