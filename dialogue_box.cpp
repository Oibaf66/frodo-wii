#include "dialogue_box.hh"

DialogueBox::DialogueBox(Font *font, const char *msgs[], int cancel) : Menu(font)
{
	this->m_selected = -1;
	this->m_cancel = cancel;

	this->setText(msgs, NULL);
}

void DialogueBox::selectCallback(int which)
{
	this->m_selected = this->p_submenus[0].sel;
}

void DialogueBox::escapeCallback(int which)
{
	this->m_selected = this->m_cancel;
}
