#include "stdafx.h"

//don't compile widgets, this is unnecesary and breaks on Linux if X11 is not present
#define DLIB_NO_GUI_SUPPORT
#include "dlib/all/source.cpp"
