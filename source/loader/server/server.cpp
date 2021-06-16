#include <SFML/Network.hpp>
#include <shared/constants.h>
#include <shared/console.h>
#include <shared/string_utils.h>
#include <Windows.h>
#include "session_manager/session_manager.h"

int main() {
	DWORD previous_mode;
	GetConsoleMode( GetStdHandle( STD_INPUT_HANDLE ), &previous_mode );
	SetConsoleMode( GetStdHandle( STD_INPUT_HANDLE ), ENABLE_EXTENDED_FLAGS | ( previous_mode & ~ENABLE_QUICK_EDIT_MODE ) );

	sf::TcpListener listener;
	if ( listener.listen( NETWORK_PORT ) != sf::Socket::Done ) {
		console::log( FMT("failed to listen on port %i \n", NETWORK_PORT) );
	}

	console::log( "server started successfully \n" );

	for ( ;; ) {
		std::shared_ptr<session> cur_session = g_session_manager->create_session( );
		if ( listener.accept( cur_session->m_socket ) != sf::Socket::Done) {
			console::log( "failed to create new client \n" );
		}

		cur_session->m_ip = cur_session->m_socket.getRemoteAddress( ).getPublicAddress( ).toString( );

		if ( !cur_session->start( ) ) {
			console::log( "failed to start session \n" );
		}
	}
}