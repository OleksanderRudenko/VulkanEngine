#pragma once

#include <cstdint>

#if defined(_WINDOWS) && defined(_USRDLL)
#	ifdef ENGINE_EXPORTS
#		define ENGINE_API __declspec(dllexport)
#	else
#		define ENGINE_API __declspec(dllimport)
#	endif
#else
#	define ENGINE_API
#endif

// Engine-wide constants
namespace xengine
{
	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
}