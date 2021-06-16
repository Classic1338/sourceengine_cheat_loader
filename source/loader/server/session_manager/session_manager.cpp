#include "session_manager.h"

std::unique_ptr< session_manager > g_session_manager = std::make_unique<session_manager>( );

std::shared_ptr< session > session_manager::create_session( ) {
	session_data.emplace_front( std::make_unique<session>( ) );
	return session_data.front( );
}

void session_manager::destroy_session( std::shared_ptr<session> a_session ) {
	a_session->m_terminated = true;
	a_session->m_socket.disconnect( );
}

std::shared_ptr<session> session_manager::get_front( )
{
	return session_data.front( );
}
