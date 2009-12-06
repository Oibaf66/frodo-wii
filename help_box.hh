#include "menu.hh"

class HelpBox : public Menu
{
public:
	HelpBox(Font *font, const char ***all_messages) : Menu(font)
	{
		this->all_messages = all_messages;
	}

	void updateHelpMessage(int which)
	{
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
