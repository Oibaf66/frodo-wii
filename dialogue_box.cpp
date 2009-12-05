#include "dialogue_box.hh"

DialogueBox::DialogueBox(Font *font, const char *msgs[], int cancel) : Menu(font)
{
	this->m_selected = -1;
	this->m_cancel = cancel;

	this->setText(msgs, NULL);
	/* Place on the second to last entry */
	this->cur_sel = this->n_entries - 2;
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
	this->m_selected = this->p_submenus[0].sel;
}

void DialogueBox::hoverCallback(int which)
{
}

void DialogueBox::escapeCallback(int which)
{
	this->m_selected = this->m_cancel;
}
