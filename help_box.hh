#ifndef __HELP_BOX_HH__
#define __HELP_BOX_HH__

#include "menu.hh"

class HelpBox : public Menu
{
public:
	HelpBox(Font *font, const char ***all_messages) : Menu(font)
	{
		this->setHelpMessages(all_messages);
		this->setSelectedBackground(NULL, NULL, NULL, NULL, NULL, NULL);
	}

	void setHelpMessages(const char ***all_messages)
	{
		this->all_messages = all_messages;
	}

	void updateHelpMessage(int which)
	{
		if (!this->all_messages)
			return;
		this->setText(this->all_messages[which]);
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

protected:
	const char ***all_messages;
};

#endif /* __HELP_BOX_HH__ */
