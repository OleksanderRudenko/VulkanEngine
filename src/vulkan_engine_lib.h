#pragma once

#if defined(_WINDOWS) && defined(_USRDLL)
#	ifdef ENGINE_EXPORTS
#		define ENGINE_API __declspec(dllexport)
#	else
#		define ENGINE_API __declspec(dllimport)
#	endif
#else
#	define ENGINE_API
#endif