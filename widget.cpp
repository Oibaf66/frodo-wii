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

void Widget::pushEvent(SDL_Event *ev)
{
	switch(ev->type)
	{
	case SDL_KEYDOWN:
		switch (ev->key.keysym.sym)
		{
		case SDLK_UP:
			this->pushEvent(KEY_UP);
			break;
		case SDLK_DOWN:
			this->pushEvent(KEY_DOWN);
			break;
		case SDLK_LEFT:
			this->pushEvent(KEY_LEFT);
			break;
		case SDLK_RIGHT:
			this->pushEvent(KEY_RIGHT);
			break;
		case SDLK_PAGEDOWN:
			this->pushEvent(KEY_PAGEDOWN);
			break;
		case SDLK_PAGEUP:
			this->pushEvent(KEY_PAGEUP);
			break;
		case SDLK_RETURN:
		case SDLK_SPACE:
			this->pushEvent(KEY_SELECT);
			break;
		case SDLK_HOME:
		case SDLK_ESCAPE:
			this->pushEvent(KEY_ESCAPE);
			break;
		default:
			break;
		}
		default:
			break;

	}
}
