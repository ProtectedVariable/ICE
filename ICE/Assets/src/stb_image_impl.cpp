// The single translation unit that compiles the stb_image implementation for the whole
// engine. Every other file includes <stb/stb_image.h> as a declaration-only header, so
// the implementation must be defined here exactly once to avoid duplicate symbols.
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
