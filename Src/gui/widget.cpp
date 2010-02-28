#include "widget.hh"

Widget::Widget()
{
	memset(this->event_stack, 0, sizeof(this->event_stack));
	this->ev_head = this->ev_tail = 0;
}

event_t Widget::popEvent()
{
	event_t out;

	if (this->ev_head == this->ev_tail)
		return EVENT_NONE;
	out = this->event_stack[this->ev_tail];
	this->ev_tail = (this->ev_tail + 1) % 8;

	return out;
}

void Widget::pushEvent(event_t ev)
{
	/* Push... */
	this->event_stack[this->ev_head] = ev;

	/* ... and update */
	this->ev_head = (this->ev_head + 1) % 8;
	if (this->ev_head == this->ev_tail)
		this->ev_tail = (this->ev_tail + 1) % 8;
}

void Widget::draw(SDL_Surface *where, int x, int y, int w, int h)
{
}
