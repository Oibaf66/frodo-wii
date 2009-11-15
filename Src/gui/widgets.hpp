/**
 * Code to populate a global Gui object with all the widgets
 * of Guichan.
 */

namespace widgets
{
gcn::ImageFont* font;
gcn::ImageFont* title_font;
gcn::Container* top;
gcn::Button* button;
gcn::TextField* textField;
gcn::TextBox* textBox;
gcn::ScrollArea* textBoxScrollArea;
gcn::ListBox* listBox;
gcn::DropDown* dropDown;
gcn::CheckBox* checkBox1;
gcn::CheckBox* checkBox2;
gcn::Window *window;
gcn::Image *darkbitsImage;
gcn::Icon* darkbitsIcon;
gcn::TabbedArea* tabbedArea;
gcn::Button* tabOneButton;
gcn::CheckBox* tabTwoCheckBox;


class BackgroundContainer : public gcn::Container
{
public:
	BackgroundContainer(gcn::ImageFont *fnt, const char *bg, const char *title) : gcn::Container()
	{
		this->bg = new gcn::Icon(bg);
		this->fnt = fnt;
		// We set the dimension of the top container to match the screen.
		this->setDimension(gcn::Rectangle(0, 0, 640, 480));
		this->title = new gcn::Label(title);
		this->title->setFont(fnt);
		this->add(this->title);
		this->title->adjustSize();
		this->title->setPosition(140, 0);
	}

	void draw(gcn::Graphics *graphics)
	{
		this->bg->draw(graphics);
		this->drawChildren(graphics);
	}

private:
	gcn::Icon *bg;
	gcn::Font *fnt;
	gcn::Label *title;
};


class List : public gcn::Widget
{
public:
	class PrivListModel : public gcn::ListModel
	{
	public:
		PrivListModel(const char **msgs, int n_msgs) : msgs(msgs), n_msgs(n_msgs)
		{
		}

		int getNumberOfElements()
		{
			return this->n_msgs;
		}

		std::string getElementAt(int i)
		{
			return this->msgs[this->n_msgs];
		}

		const char **msgs;
		int n_msgs;
	};

	List(const char **msgs, int n_msgs) {
	}
};

/*
 * List boxes and drop downs need an instance of a list model
 * in order to display a list.
 */
 class DemoListModel : public gcn::ListModel
 {
 public:
	 int getNumberOfElements()
	 {
		 return 5;
	 }

	 std::string getElementAt(int i)
	 {
		 switch(i)
		 {
		 case 0:
			 return std::string("zero");
		 case 1:
			 return std::string("one");
		 case 2:
			 return std::string("two");
		 case 3:
			 return std::string("three");
		 case 4:
			 return std::string("four");
		 case 5:
			 return std::string("four");
		 case 6:
			 return std::string("four");
		 case 7:
			 return std::string("four");
		 default: // Just to keep warnings away
			 return std::string("");
		 }
	 }
 };

 DemoListModel demoListModel;

 /**
  * Initialises the widgets example by populating the global Gui
  * object.
  */
 void init()
 {
	 // Now we load the font used in this example.
	 font = new gcn::ImageFont("data/fixedfont.bmp", " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	 // Widgets may have a global font so we don't need to pass the
	 // font object to every created widget. The global font is static.
	 gcn::Widget::setGlobalFont(font);

	 title_font = new gcn::ImageFont("data/techyfontbig.png", " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	 // We first create a container to be used as the top widget.
	 // The top widget in Guichan can be any kind of widget, but
	 // in order to make the Gui contain more than one widget we
	 // make the top widget a container.
	 top = new BackgroundContainer(title_font, "data/menu_background.png", "Main menu");
	 // Finally we pass the top widget to the Gui object.
	 globals::gui->setTop(top);

	 // Now we create the widgets
	 button = new gcn::Button("Button");

	 textField = new gcn::TextField("Text field");

	 textBox = new gcn::TextBox("Multiline\nText box");
	 textBoxScrollArea = new gcn::ScrollArea(textBox);
	 textBoxScrollArea->setWidth(200);
	 textBoxScrollArea->setHeight(100);
	 textBoxScrollArea->setFrameSize(1);

	 listBox = new gcn::ListBox(&demoListModel);
	 listBox->setFrameSize(1);
	 dropDown = new gcn::DropDown(&demoListModel);

	 checkBox1 = new gcn::CheckBox("Checkbox 1");
	 checkBox2 = new gcn::CheckBox("Checkbox 2");

	 window = new gcn::Window("I am a window  Drag me");
	 window->setBaseColor(gcn::Color(255, 150, 200, 190));

	 darkbitsImage = gcn::Image::load("data/darkbitslogo_by_haiko.bmp");
	 darkbitsIcon = new gcn::Icon(darkbitsImage);
	 window->add(darkbitsIcon);
	 window->resizeToContent();

	 tabbedArea = new gcn::TabbedArea();
	 tabbedArea->setSize(200, 100);
	 tabOneButton = new gcn::Button("A button in tab 1");
	 tabbedArea->addTab("Tab 1", tabOneButton);
	 tabTwoCheckBox = new gcn::CheckBox("A check box in tab 2");
	 tabbedArea->addTab("Tab 2", tabTwoCheckBox);

	 // Now it's time to add the widgets to the top container
	 // so they will be conected to the GUI.
	 top->add(button, 200, 80);
	 top->add(textField, 250, 80);
	 top->add(textBoxScrollArea, 200, 50);
	 top->add(listBox, 200, 200);
	 top->add(dropDown, 500, 80);
	 top->add(checkBox1, 500, 130);
	 top->add(checkBox2, 500, 150);
	 top->add(window, 50, 350);
	 top->add(tabbedArea, 400, 350);
 }

 /**
  * Halts the widgets example.
  */
 void halt()
 {
	 delete font;
	 delete title_font;
	 delete top;
	 delete button;
	 delete textField;
	 delete textBox;
	 delete textBoxScrollArea;
	 delete listBox;
	 delete dropDown;
	 delete checkBox1;
	 delete checkBox2;
	 delete window;
	 delete darkbitsIcon;
	 delete darkbitsImage;
	 delete tabbedArea;
	 delete tabOneButton;
	 delete tabTwoCheckBox;
 }
}
