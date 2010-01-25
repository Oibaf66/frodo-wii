#include "status_bar.hh"
#include "gui.hh"

StatusBar::StatusBar() : Menu(Gui::gui->small_font), TimeoutHandler()
{
	memset(this->messages, 0, sizeof(this->messages));
	this->head = this->tail = 0;
	this->cur_message = NULL;
	this->setSelectedBackground(NULL, NULL, NULL, NULL, NULL, NULL);
}


void StatusBar::queueMessage(const char *message)
{
	this->messages[this->head] = message;

	/* If this is the first message, display it as soon as possible */
	if (this->head == this->tail)
		Gui::gui->timerController->arm(this, 1);

	this->head = (this->head + 1) % N_STATUS_MESSAGES;
	if (this->head == this->tail)
		this->tail = (this->tail + 1) % N_STATUS_MESSAGES;
}

const char *StatusBar::dequeueMessage()
{
	const char *out = this->messages[this->tail];

	if (this->head == this->tail)
		return NULL;
	this->tail = (this->tail + 1) % N_STATUS_MESSAGES;

	return out;
}

void StatusBar::timeoutCallback()
{
	static const char *text[2];

	this->cur_message = this->dequeueMessage();
	text[0] = this->cur_message;
	text[1] = NULL;

	if (this->cur_message)
	{
		Gui::gui->timerController->arm(this, 2000);
		this->setText(text);
	}
	else
		this->setText(NULL);
}

void StatusBar::draw(SDL_Surface *where)
{
	SDL_Rect dst;
	int x = 130;
	int y = 12;
	int w = 496;
	int h = 56;

	if (!this->cur_message)
		return;

	/* Blit the backgrounds */
	dst = (SDL_Rect){x,y,0,0};
	SDL_BlitSurface(Gui::gui->status_bar_bg, NULL, where, &dst);

	Menu::draw(where, x+4, y+4, w, h);
}
