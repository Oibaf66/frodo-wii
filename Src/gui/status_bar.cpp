#include "status_bar.hh"
#include "gui.hh"
#include "utils.hh"

StatusBar::StatusBar() : Menu(Gui::gui->small_font), TimeoutHandler()
{
	memset(this->messages, 0, sizeof(this->messages));
	this->head = this->tail = 0;
	this->cur_message = NULL;
	this->setSelectedBackground(NULL, NULL, NULL, NULL, NULL, NULL);
}


void StatusBar::queueMessage(const char *fmt, ...)
{
	char buf[255];
	va_list ap;
	int r;

	memset(buf, 0, sizeof(buf));
	va_start(ap, fmt);
	r = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
	va_end(ap);

	/* Free the existing message if we are overwriting it */
	free((void*)this->messages[this->head]);

	this->messages[this->head] = xstrdup(buf);

	/* If this is the first message, display it as soon as possible */
	if (this->head == this->tail)
		TimerController::controller->arm(this, 1);

	this->head = (this->head + 1) % N_STATUS_MESSAGES;
	if (this->head == this->tail)
		this->tail = (this->tail + 1) % N_STATUS_MESSAGES;
}

const char *StatusBar::dequeueMessage()
{
	const char *out = this->messages[this->tail];

	if (this->head == this->tail)
		return NULL;
	this->messages[this->tail] = NULL;
	this->tail = (this->tail + 1) % N_STATUS_MESSAGES;

	return out;
}

void StatusBar::timeoutCallback()
{
	const char *text[2];

	this->cur_message = this->dequeueMessage();
	text[0] = this->cur_message;
	text[1] = NULL;

	if (this->cur_message)
		this->setText(text);
	else
		this->setText(NULL);
	TimerController::controller->arm(this, 2000);
	free((void *)this->cur_message);
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
