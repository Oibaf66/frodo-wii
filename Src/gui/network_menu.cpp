#include "gui.hh"
#include "menu.hh"
#include "help_box.hh"
#include "virtual_keyboard.hh"

#include <sysdeps.h>
#include <C64.h>

#ifdef GEKKO
#include <smb.h>
#endif

/****************************************************************************
 * Mount SMB Share
 ****************************************************************************/
extern bool smbismount;

bool ConnectShare ()
{
	
	if(smbismount)
		return true;
		
		#ifdef GEKKO
		if(smbInit(ThePrefs.SmbUser, ThePrefs.SmbPwd,ThePrefs.SmbShare, ThePrefs.SmbIp))
			smbismount = true;
		#endif	

		if(!smbismount) Gui::gui->status_bar->queueMessage("Failed to connect to SMB share");
		else {
		Gui::gui->status_bar->queueMessage("Established connection to SMB share");
		Gui::gui->np->Port = PORT_SMB;
		}

	return smbismount;
}


void CloseShare(bool silent)
{

	if(smbismount) {
	if (!silent) Gui::gui->status_bar->queueMessage("Disconnected from SMB share");
	#ifdef GEKKO
	smbClose("smb");
	#endif
	}
	smbismount = false;
}


extern bool networkisinit; 

class NetworkView;

class NetworkMenu : public Menu, public KeyboardListener
{
	friend class NetworkView;

public:
	NetworkMenu(Font *font, HelpBox *help) : Menu(font)
	{
		this->help = help;
		memset(this->messages, 0, sizeof(this->messages));
		memset(this->strs, 0, sizeof(this->strs));
	}

	~NetworkMenu()
	{
	}

	virtual void stringCallback(const char *str)
	{
		switch (this->cur_sel)
		{
		case 0:
			strncpy(Gui::gui->np->NetworkName, str, sizeof(Gui::gui->np->NetworkName));
			break;
		case 1:
			strncpy(Gui::gui->np->NetworkServer, str, sizeof(Gui::gui->np->NetworkName));
			break;
		case 2:
		{
			char *endp;
			unsigned long v;

			v = strtoul(str, &endp, 0);
			if (endp == str)
			{
				DialogueBox *error_dialogue = new DialogueBox(network_port_dialogue_messages);
				Gui::gui->pushDialogueBox(error_dialogue);
			}
			else
				Gui::gui->np->NetworkPort = v;
		} break;
		case 9:
			strncpy(Gui::gui->np->SmbUser, str, sizeof(Gui::gui->np->SmbUser));
			break;
		case 10:
			strncpy(Gui::gui->np->SmbPwd, str, sizeof(Gui::gui->np->SmbPwd));
			break;
		case 11:
			strncpy(Gui::gui->np->SmbShare, str, sizeof(Gui::gui->np->SmbShare));
			break;
		case 12:
			if (!inet_aton(str, NULL)) 
			{
				DialogueBox *error_dialogue = new DialogueBox(network_bad_ip_dlg);
				Gui::gui->pushDialogueBox(error_dialogue);
			}
			else strncpy(Gui::gui->np->SmbIp, str, sizeof(Gui::gui->np->SmbIp));
			break;
		default:
			panic("Cur sel is %d, not possible!\n", this->cur_sel);
			break;
		}
		this->updateMessages();
	}
	
	virtual void selectCallback(int which)
	{
		//printf("option entry %d selected: %s\n", which, this->pp_msgs[which]);
		switch (which)
		{
		case 0:
		case 1:
			VirtualKeyboard::kbd->activate();
			VirtualKeyboard::kbd->registerListener(this);
			break;
		case 2:
			Gui::gui->pushView(Gui::gui->nrv);
			break;
		case 4:
			if (!networkisinit) Gui::gui->pushDialogueBox(new DialogueBox(network_is_not_init_dlg));
			else if (TheC64->network)
				TheC64->network->Disconnect();
			else if ( strncmp(Gui::gui->np->NetworkName, "Unset", strlen("Unset")) == 0)
				Gui::gui->pushDialogueBox(new DialogueBox(network_unset_name_dlg));
			else
			{
				TheC64->network = new Network(Gui::gui->np->NetworkServer,
						Gui::gui->np->NetworkPort);
				TheC64->network_connection_type = CONNECT;
				Gui::gui->exitMenu();
				TheC64->network->ConnectToBroker();
			}
			break;
		case 6:
			if (TheC64->network_connection_type == NONE)
				Gui::gui->pushDialogueBox(new DialogueBox(network_need_connection));
			else
			{
				Gui::gui->exitMenu();
				TheC64->TheDisplay->TypeNetworkMessage(true);
			}
			break;
		case 7:
			if (TheC64->network_connection_type != MASTER &&
					TheC64->network_connection_type != CLIENT)
				Gui::gui->pushDialogueBox(new DialogueBox(network_need_peer));
			else
			{
				Gui::gui->exitMenu();
				TheC64->TheDisplay->TypeNetworkMessage();
			}
			break;
		case 9:
		case 10:
		case 11:
		case 12:
			VirtualKeyboard::kbd->activate();
			VirtualKeyboard::kbd->registerListener(this);
			break;
		case 13:
			if (!networkisinit) Gui::gui->pushDialogueBox(new DialogueBox(network_is_not_init_dlg));
			else {if (smbismount) CloseShare(false); else ConnectShare();this->updateMessages();}
			break;	
			
		default:
			break;
		}
	}

	virtual void hoverCallback(int which)
	{
		this->help->updateHelpMessage(which);
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->popView();
	}

private:
	void updateMessages()
	{
		memset(this->strs, 0, sizeof(this->strs));
		snprintf(this->strs[0], sizeof(this->strs[0]) - 1, "Set username (%s)",
				Gui::gui->np->NetworkName);
		snprintf(this->strs[1], sizeof(this->strs[1]) - 1, "Server (%s)",
				Gui::gui->np->NetworkServer);
		snprintf(this->strs[2], sizeof(this->strs[2]) - 1, "Set region (%s)",
				region_to_str(Gui::gui->np->NetworkRegion));
		snprintf(this->strs[3], sizeof(this->strs[3]) - 1, "Set SMB user (%s)",
				Gui::gui->np->SmbUser);
		snprintf(this->strs[4], sizeof(this->strs[4]) - 1, "Set SMB psw (%s)",
				Gui::gui->np->SmbPwd);
		snprintf(this->strs[5], sizeof(this->strs[5]) - 1, "Set SMB folder (%s)",
				Gui::gui->np->SmbShare);		
		snprintf(this->strs[6], sizeof(this->strs[6]) - 1, "Set SMB IP (%s)",
				Gui::gui->np->SmbIp);
				
		this->messages[0] = this->strs[0];
		this->messages[1] = this->strs[1];
		this->messages[2] = this->strs[2];

		this->messages[3] = " ";
		this->messages[4] = TheC64->network ? "Disconnect C64 network" : "Connect to C64 network!";
		this->messages[5] = " ";
		this->messages[6] = "Post network message";
		this->messages[7] = "Post peer message";
		this->messages[8] = " ";
		this->messages[9] = this->strs[3];
		this->messages[10] = this->strs[4];
		this->messages[11] = this->strs[5];
		this->messages[12] = this->strs[6];
		this->messages[13] = smbismount ? "Disconnect from SMB share" : "Connect to SMB share";
		this->messages[14] = NULL;
		this->setText(this->messages);
	}

	char strs[7][255];
	const char *messages[15];

	HelpBox *help;
};


class NetworkView : public GuiView
{
public:
	NetworkView() : GuiView()
	{
		this->help = new HelpBox(Gui::gui->small_font, network_menu_help);
		this->menu = new NetworkMenu(Gui::gui->default_font, this->help);
	}

	~NetworkView()
	{
		delete this->help;
		delete this->menu;
	}

	void runLogic()
	{
		this->menu->runLogic();
	}

	void pushEvent(event_t ev)
	{
		this->menu->pushEvent(ev);
	}

	void viewPushCallback()
	{
		this->menu->updateMessages();
	}

	void draw(SDL_Surface *where)
	{
		 SDL_Rect dst;

		 /* Blit the backgrounds */
		 dst = (SDL_Rect){20,45,300,400};
		 SDL_BlitSurface(Gui::gui->main_menu_bg, NULL, where, &dst);

		 dst = (SDL_Rect){350,13,0,0};
		 SDL_BlitSurface(Gui::gui->infobox, NULL, where, &dst);

		 this->menu->draw(where, 50, 70, 300, 400);
		 this->help->draw(where, 354, 24, 264, 210);
		 Gui::gui->server_msgs->draw(where);
	}

protected:
	NetworkMenu *menu;
	HelpBox *help;
};
