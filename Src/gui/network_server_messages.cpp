#include "network_server_messages.hh"
#include "utils.hh"

NetworkServerMessages::NetworkServerMessages() : StatusBar()
{
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(this->flowed_messages) - 1; i++)
		this->flowed_messages[i] = (char *)xmalloc(28);
	this->flowed_messages[i] = NULL;
}

NetworkServerMessages::~NetworkServerMessages()
{
	for (unsigned i = 0; i < ARRAY_SIZE(this->flowed_messages) - 1; i++)
		free(this->flowed_messages[i]);
}

void NetworkServerMessages::draw(SDL_Surface *where)
{
	SDL_Rect dst;
	int x = 350;
	int y = 242;
	int w = Gui::gui->network_message_box->w;
	int h = Gui::gui->network_message_box->h;

	dst = (SDL_Rect){x,y,0,0};
	SDL_BlitSurface(Gui::gui->network_message_box, NULL, where, &dst);

	Menu::draw(where, x+6, y+8, w-6, h-8);
}

void NetworkServerMessages::timeoutCallback()
{
	const char *cur = this->messages[this->tail];
	char *cpy = xstrdup(cur);
	char *msgp;
	char *p;
	size_t line_len = 0;
	size_t line = 0;

	for (unsigned i = 0; i < ARRAY_SIZE(this->flowed_messages) - 1; i++)
		this->flowed_messages[i][0] = '\0';

	p = strtok_r(cpy, " ", &msgp);
	while (p)
	{
		int word_len = strlen(p);

		if (line_len + word_len > 24)
		{
			line++;
			/* Too long! */
			if (line >= ARRAY_SIZE(this->flowed_messages))
				break;
			line_len = 0;
		}
		strcat(this->flowed_messages[line], p);
		strcat(this->flowed_messages[line], " ");

		line_len += word_len + 1;
		p = strtok_r(NULL, " ", &msgp);
	}
	for (line = line + 1; line < ARRAY_SIZE(this->flowed_messages) - 1; line++)
		strcpy(this->flowed_messages[line], "#");

	this->setText((const char **)this->flowed_messages);

	while (1)
	{
		this->tail = (this->tail + 1) % N_STATUS_MESSAGES;
		if (this->messages[this->tail])
			break;
	}
	TimerController::controller->arm(this, 5000);

	free(cpy);
}
