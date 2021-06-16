#pragma once
#include <string>

namespace console {
	static void log( std::string_view string ) {
		printf( string.data( ) );
	}
}