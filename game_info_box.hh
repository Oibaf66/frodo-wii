#include "menu.hh"
#include "game_info.hh"

class GameInfoBox : public Menu
{
public:
	GameInfoBox(Font *font) : Menu(font)
	{
		this->gi = NULL;
		memset(this->gi_messages, 0, sizeof(this->gi_messages));
		this->setSelectedBackground(NULL, NULL, NULL, NULL, NULL, NULL);
	}

	void setGameInfo(GameInfo *gi)
	{
		/* Make a copy */
		this->gi = new GameInfo(gi);
	}

	void loadGameInfo(const char *what)
	{
		this->setText(NULL);
		memset(this->gi_messages, 0, sizeof(this->gi_messages));

		/* Reset the current game info */
		if (this->gi)
		{
			delete this->gi;
			this->gi = NULL;
		}

		/* No need to do this for directories or the special "None" field */
		if (strcmp(what, "None") == 0 ||
				what[0] == '[')
			return;

		size_t len = strlen(Gui::gui->metadata_base_path) + strlen(what) + 6;
		char *tmp = (char*)xmalloc(len);
		sprintf(tmp, "%s/%s.lra", Gui::gui->metadata_base_path, what);

		/* Might return NULL, but that's OK */
		this->gi = GameInfo::loadFromFile(tmp);
		if (this->gi)
		{
			this->gi_messages[0] = "Game:";
			this->gi_messages[1] = this->gi->name;
			this->gi_messages[2] = "Author:";
			this->gi_messages[3] = this->gi->author;
			this->setText(this->gi_messages);
		}

		free(tmp);
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
		if (!this->gi->screenshot)
			return;

		SDL_Rect dst;

		/* Blit the backgrounds */
		dst = (SDL_Rect){x + w / 2 - this->gi->screenshot->w / 2, y, w, h};
		SDL_BlitSurface(this->gi->screenshot, NULL, where, &dst);

		Menu::draw(where, x, y + this->gi->screenshot->h + 10,
				w, h - this->gi->screenshot->h - 10);
	}

private:
	const char *gi_messages[6];
	GameInfo *gi;
};
