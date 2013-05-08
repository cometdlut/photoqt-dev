#ifndef SETUP_WIDGETS_H
#define SETUP_WIDGETS_H

// Some global variables
class SetupWidgets {

public:

	// Initially all these widgets aren't set up
	SetupWidgets() {
		filehandling = false;
		startup = false;
		about = false;
		wallpaper = false;
		slideshow = false;
		slideshowbar = false;
		settings = false;
	}

	bool filehandling;
	bool startup;
	bool about;
	bool wallpaper;
	bool slideshow;
	bool slideshowbar;
	bool settings;

};


#endif // SETUP_WIDGETS_H