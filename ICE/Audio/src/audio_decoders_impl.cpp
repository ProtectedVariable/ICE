// Single translation unit that instantiates the header-only audio decoders, mirroring the
// stb_image_impl.cpp precedent in the assets module. Nothing else in the engine may define these
// implementation macros -- doing so would produce duplicate symbols at link time.
//
// OpenAL is playback-only (it accepts raw PCM and nothing else), so decoding is the engine's
// responsibility. AudioDecoder.cpp is the only consumer of these.

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

#define DR_MP3_IMPLEMENTATION
#include <dr_mp3.h>

#define DR_FLAC_IMPLEMENTATION
#include <dr_flac.h>

// stb_vorbis ships as a .c file. It is compiled here as part of this TU so the whole decoder set
// is confined to one object file. STB_VORBIS_NO_STDIO is deliberately NOT set: the loader decodes
// straight from a path.
#define STB_VORBIS_NO_PUSHDATA_API
#include <stb_vorbis.c>
