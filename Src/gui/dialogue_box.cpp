#include "dialogue_box.hh"

void DialogueListener::escapeCallback(DialogueBox *which, int selected)
{
	Gui::gui->popDialogueBox();
}

void DialogueListener::selectCallback(DialogueBox *which, int selected)
{
	Gui::gui->popDialogueBox();
}

DialogueBox::DialogueBox(const char *msgs[], bool delete_on_action) : Menu(NULL), ListenerManager()
{
	this->setFont(Gui::gui->default_font);
	this->setSelectedBackground(NULL, NULL, NULL,
			Gui::gui->bg_left, Gui::gui->bg_middle,
			Gui::gui->bg_right);

	this->setText(msgs, NULL);
	/* Place on the second to last entry */
	this->cur_sel = this->n_entries - 2;
	this->delete_on_action = delete_on_action;
}

int DialogueBox::selectNext(event_t ev)
{
	/* No up/down movement please! */
	if (ev == KEY_UP || ev == KEY_DOWN)
		return this->cur_sel;
	return Menu::selectNext(ev);
}

void DialogueBox::selectCallback(int which)
{
	for (int i = 0; i < this->nListeners(); i++)
		if (this->listeners[i])
			((DialogueListener*)this->listeners[i])->selectCallback(this,
					this->p_submenus[0].sel);
	Gui::gui->popDialogueBox();
	if (this->delete_on_action)
		delete this;
}

void DialogueBox::hoverCallback(int which)
{
}

void DialogueBox::escapeCallback(int which)
{
	for (int i = 0; i < this->nListeners(); i++)
		if (this->listeners[i])
			((DialogueListener*)this->listeners[i])->escapeCallback(this,
					this->p_submenus[0].sel);
	Gui::gui->popDialogueBox();
	if (this->delete_on_action)
		delete this;
}

void DialogueBox::draw(SDL_Surface *where)
{
	 int d_x = where->w / 2 - Gui::gui->dialogue_bg->w / 2;
	 int d_y = where->h / 2 - Gui::gui->dialogue_bg->h / 2;

	 SDL_Rect dst = (SDL_Rect){d_x, d_y,
		 Gui::gui->dialogue_bg->w, Gui::gui->dialogue_bg->h};
	 SDL_BlitSurface(Gui::gui->dialogue_bg, NULL, where, &dst);

	 Menu::draw(where, d_x + 10, d_y + 10,
			 Gui::gui->dialogue_bg->w - 10, Gui::gui->dialogue_bg->h - 10);
}

