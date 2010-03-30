#ifndef _GAME_INFO_BOX_H_
#define _GAME_INFO_BOX_H_

#include "menu.hh"
#include "game_info.hh"

class GameInfoBox : public Menu
{
public:
	GameInfoBox(Font *font) : Menu(font)
	{
		this->gi = NULL;
		memset(this->year, 0, sizeof(this->year));
		memset(this->players, 0, sizeof(this->players));
		memset(this->gi_messages, 0, sizeof(this->gi_messages));
		this->setSelectedBackground(NULL, NULL, NULL, NULL, NULL, NULL);
	}

	void setGameInfo(GameInfo *gi)
	{
		/* Make a copy */
		this->gi = gi;
		this->updateMessages();
	}

	void loadGameInfo(const char *what, const char *base_path)
	{
		/* No need to do this for directories or the special "None" field */
		if ( !(strcmp(what, "None") == 0 ||
				what[0] == '[') )
		{
			size_t len = strlen(base_path) + strlen(what) + 6;
			char *tmp = (char*)xmalloc(len);

			sprintf(tmp, "%s/%s.lra", base_path, what);

			/* Might return NULL, but that's OK */
			this->gi = GameInfo::loadFromFile(tmp);
			free(tmp);
		}
		this->updateMessages();
	}

	virtual void selectCallback(int which)
	{
	}
	virtual void hoverCallback(int which)
	{
	}
	virtual void escapeCallback(int which)
	{
	}

	void draw(SDL_Surface *where, int x, int y, int w, int h)
	{
		if (!this->gi)
			return;
		if (this->gi->screenshot)
		{
			SDL_Rect dst;

			/* Blit the screenshot */
			dst = (SDL_Rect){x + w / 2 - this->gi->screenshot->w / 2, y, w, h};
			SDL_BlitSurface(this->gi->screenshot, NULL, where, &dst);

			Menu::draw(where, x + 20, y + this->gi->screenshot->h + 10,
					w - 20, h - this->gi->screenshot->h - 10);
		}
		else
			Menu::draw(where, x + 20, y + 288 / 2 + 2, w - 20, h - 10);
	}

	void updateMessages()
	{
		this->setText(NULL);

		for (unsigned i = 0; i < ARRAY_SIZE(this->gi_messages) - 1; i++)
			this->gi_messages[i] = " ";
		this->gi_messages[ARRAY_SIZE(this->gi_messages) - 1] = NULL;

		if (this->gi)
		{
			snprintf(this->year, sizeof(this->year), "%d", this->gi->year);
			snprintf(this->players, sizeof(this->players), "%d", this->gi->players);
			this->gi_messages[0] = this->gi->name ? this->gi->name : " ";
			this->gi_messages[1] = this->gi->publisher ? this->gi->publisher : " ";
			this->gi_messages[2] = this->gi->creator ? this->gi->creator : " ";
			this->gi_messages[3] = this->gi->musician ? this->gi->musician : " ";
			this->gi_messages[4] = this->gi->graphics_artist ? this->gi->graphics_artist : " ";
			if (this->gi->genre == GENRE_UNKNOWN || this->gi->genre >= GENRE_MAX)
				this->gi_messages[5] = "Unknown";
			else
				this->gi_messages[5] = genre_dlg[this->gi->genre - 1];
		}
		this->gi_messages[6] = year;
		this->gi_messages[7] = this->players;

		this->setText(this->gi_messages);
	}

	const char *gi_messages[10];
	char year[8];
	char players[4];
	GameInfo *gi;
};

#endif /* _GAME_INFO_BOX_H_ */
