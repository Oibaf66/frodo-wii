/*
 * Code that sets up an SDL application with Guichan using the 
 * Guichan SDL back end.
 */

#include <guichan.hpp>
#include <guichan/sdl.hpp>

extern SDL_Surface *real_screen;
namespace sdl
{
// All back ends contain objects to make Guichan work on a
// specific target - in this case SDL - and they are a Graphics
// object to make Guichan able to draw itself using SDL, an
// input objec to make Guichan able to get user input using SDL
// and an ImageLoader object to make Guichan able to load images
// using SDL.
gcn::SDLGraphics* graphics;
gcn::SDLInput* input;
gcn::SDLImageLoader* imageLoader;

/**
 * Initialises the SDL application. This function creates the global
 * Gui object that can be populated by various examples.
 */
void init()
{
	// We want unicode for the SDLInput object to function properly.
	SDL_EnableUNICODE(1);
	// We also want to enable key repeat.
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	// Now it's time to initialise the Guichan SDL back end.

	imageLoader = new gcn::SDLImageLoader();
	// The ImageLoader Guichan should use needs to be passed to the Image object
	// using a static function.
	gcn::Image::setImageLoader(imageLoader);
	graphics = new gcn::SDLGraphics();
	// The Graphics object needs a target to draw to, in this case it's the
	// screen surface, but any surface will do, it doesn't have to be the screen.
	graphics->setTarget(real_screen);
	input = new gcn::SDLInput();

	// Now we create the Gui object to be used with this SDL application.
	globals::gui = new gcn::Gui();
	// The Gui object needs a Graphics to be able to draw itself and an Input
	// object to be able to check for user input. In this case we provide the
	// Gui object with SDL implementations of these objects hence making Guichan
	// able to utilise SDL.
	globals::gui->setGraphics(graphics);
	globals::gui->setInput(input);
}


/**
 * Runs the SDL application.
 */
void handle_input(SDL_Event event)
{
	// feed the input to Guichan by pushing the input to the Input
	// object.
	input->pushInput(event);
}

void run()
{
	// Now we let the Gui object perform its logic.
	globals::gui->logic();
}

void draw()
{
	// Now we let the Gui object draw itself.
	globals::gui->draw();
}

} /* namespace */
