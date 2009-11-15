/**
 * This is an example that shows of the widgets present in
 * Guichan. The example uses the SDL back end.
 */
#include "sysdeps.h"

#include "main.h"
#include "Prefs.h"
#include "C64.h"
#include "SAM.h"

#include <guichan.hpp>
#include <iostream>

// Here we store a global Gui object.  We make it global
// so it's easily accessable. Of course, global variables
// should normally be avioded when it comes to OOP, but
// this examples is not an example that shows how to make a 
// good and clean C++ application but merely an example
// that shows how to use Guichan.
namespace globals
{
gcn::Gui* gui;
}

// Include code to set up an SDL application with Guichan.
// The sdl.hpp file is responsible for creating and deleting
// the global Gui object.
#include "sdl.hpp"
// Include code to set up a Guichan GUI with all the widgets
// of Guichan. The code populates the global Gui object.
#include "widgets.hpp"

void gui_menu_receive_input(SDL_Event ev)
{
	if (!TheC64->is_in_menu())
		return;
	sdl::handle_input(ev);
}

int gui_menu_init()
{
	try
	{
		sdl::init();
		widgets::init();
	}
	// Catch all Guichan exceptions.
	catch (gcn::Exception e)
	{
		std::cerr << e.getMessage() << std::endl;
		return 1;
	}
	// Catch all Std exceptions.
	catch (std::exception e)
	{
		std::cerr << "Std exception: " << e.what() << std::endl;
		return 1;
	}
	// Catch all unknown exceptions.
	catch (...)
	{
		std::cerr << "Unknown exception" << std::endl;
		return 1;
	}

	return 0;
}

int gui_menu_run()
{
	if (!TheC64->is_in_menu())
		return 0;
	try
	{
		sdl::run();
	}
	// Catch all Guichan exceptions.
	catch (gcn::Exception e)
	{
		std::cerr << e.getMessage() << std::endl;
		return 1;
	}
	// Catch all Std exceptions.
	catch (std::exception e)
	{
		std::cerr << "Std exception: " << e.what() << std::endl;
		return 1;
	}
	// Catch all unknown exceptions.
	catch (...)
	{
		std::cerr << "Unknown exception" << std::endl;
		return 1;
	}

	return 0;
}

int gui_menu_draw()
{
	if (!TheC64->is_in_menu())
		return 0;
	try
	{
		sdl::draw();
	}
	// Catch all Guichan exceptions.
	catch (gcn::Exception e)
	{
		std::cerr << e.getMessage() << std::endl;
		return 1;
	}
	// Catch all Std exceptions.
	catch (std::exception e)
	{
		std::cerr << "Std exception: " << e.what() << std::endl;
		return 1;
	}
	// Catch all unknown exceptions.
	catch (...)
	{
		std::cerr << "Unknown exception" << std::endl;
		return 1;
	}

	return 0;
}
