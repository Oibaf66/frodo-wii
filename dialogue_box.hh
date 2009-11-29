#ifndef __DIALOGUE_BOX_HH__
#define __DIALOGUE_BOX_HH__

#include "menu.hh"

class DialogueBox : public Menu
{
	DialogueBox(Font *font, const char *msgs[], int cancel = 1);

	virtual void selectCallback(int which);

	virtual void escapeCallback(int which);

	int selected()
	{
		return this->m_selected;
	}

private:
	int m_selected;
	int m_cancel;
};

#endif /* __DIALOGUE_BOX_HH__ */
