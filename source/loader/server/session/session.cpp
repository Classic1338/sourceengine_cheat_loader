#include "../session_manager/session_manager.h"
#include <random>
#include <fstream>
#include "../server/dependencies/json/json.hpp"
#include "shared/constants.h"

std::vector<char> cache_module( std::string_view a_module_name) {
	std::vector<char> m_data;
	std::fstream stream( a_module_name.data( ), std::ios::in | std::ios::binary );
	if ( !stream.is_open( ) ) {
		console::log( FMT( "failed to cache %s, file not found", a_module_name.data( ) ) );
		return {};
	}

	stream.seekg( 0, std::ios::end );
	m_data.reserve( static_cast< size_t >( stream.tellg( ) ) );
	stream.seekg( 0, std::ios::beg );

	m_data.assign( ( std::istreambuf_iterator<char>( stream ) ),
		std::istreambuf_iterator<char>( ) );
	stream.close( );

	return m_data;
}

bool is_user_verified( const std::string& username, const std::string& password ) {	
	return true;
}

void handle_loader_session( std::shared_ptr<session> session ) {
	console::log( FMT( "created loader session for %s \n", session->m_ip.data( ) ) );

	std::string password;
	packet_t packet;

	sf::Socket::Status status = session->m_socket.receive(packet);
	if (status != sf::Socket::Done) {
		if (status == sf::Socket::Disconnected) {
			console::log(FMT("failed to get status of %s \n", session->m_ip.data()));
			return;
		}
	}

	packet >> session->m_username;
	packet >> password;

	printf( "user %s connected successfully \n", session->m_username.data( ) );

	packet.clear( );

	if (session->m_loader_version != LOADER_VERSION) {
		printf("%s is using an out of date loader \n", session->m_username.c_str());
		printf("client loader version: %i \n", session->m_loader_version);
		return;
	}

	get_login_info login_data( session->m_username, password, "kTaXHHJM6YnKAhXwCmjTdUqSkoZN0vKz" );
	if ( login_data.is_user( ) ) {
		packet << "logged in";
		packet << login_data.m_user_premium;
		packet << login_data.m_user_beta;
		packet << login_data.m_user_banned;
		session->m_socket.send( packet );
	}
	else {
		packet << "username or password incorrect";
		packet << false;
		packet << false;
		packet << true;
		session->m_socket.send( packet );
		return;
	}

	packet.clear( );

	int game_selection = 1;
	const sf::Socket::Status game_status = session->m_socket.receive( packet );
	if ( game_status != sf::Socket::Done ) {
		if ( game_status == sf::Socket::Disconnected ) {
			console::log( FMT( "failed to get game selection of %s \n", session->m_username.data( ) ) );
			return;
		}
	}
	packet >> game_selection;
	session->m_game_type = game_selection;
	if ( !login_data.m_user_beta )
		game_selection = 1;

	packet.clear( );
	int cheat_selection;
	const sf::Socket::Status cheat_status = session->m_socket.receive( packet );
	if ( cheat_status != sf::Socket::Done ) {
		if ( cheat_status == sf::Socket::Disconnected ) {
			console::log( FMT( "failed to get cheat selection of %s \n", session->m_username.data( ) ) );
			return;
		}
	}
	packet >> cheat_selection;
	if ( !login_data.m_user_beta )
		cheat_selection = 1;

	session->m_cheat_selection = cheat_selection;

	std::random_device random_device;
	std::mt19937_64 mt_generator( random_device( ) );
	std::uniform_int_distribution<uint64_t> distribution;
	session->m_key = distribution( mt_generator );

	std::string module_name;
	switch ( game_selection ) {
	case 0: break;
	case 1:
		switch ( cheat_selection ) {
		case 0:
			break;
		case 1:
			module_name = "C:\\Users\\tyler\\Documents\\GitHub\\csgo-cheat-loader\\compiled\\server\\dlls\\csgo_release.dll";
			break;
		case 2:
			module_name = "C:\\Users\\tyler\\Documents\\GitHub\\csgo-cheat-loader\\compiled\\server\\dlls\\csgo_beta.dll";
			break;
		}
		break;
	case 2:
		switch ( cheat_selection ) {
		case 0:
			break;
		case 1:
			module_name = "C:\\Users\\tyler\\Documents\\GitHub\\csgo-cheat-loader\\compiled\\server\\dlls\\l4d2_release.dll";
			break;
		case 2:
			module_name = "C:\\Users\\tyler\\Documents\\GitHub\\csgo-cheat-loader\\compiled\\server\\dlls\\l4d2_beta.dll";
			break;
		}
		break;
	}

	packet.clear( );

	packet << session->m_key << cache_module( module_name );
	status = session->m_socket.send( packet );
	if ( status != sf::Socket::Done ) {
		if ( status == sf::Socket::Disconnected ) {
			console::log( FMT( "failed to send module to user: %s \n", session->m_username.data( ) ) );
			return;
		}
	}
}

void handle_injected_session( std::shared_ptr<session> session ) {
	console::log( "created injected session \n" );
}

bool session::start( ) {
	m_thread = std::make_shared<std::thread>( [ & ]( std::shared_ptr<session> session ) {
		packet_t packet;
		const sf::Socket::Status status = session->m_socket.receive( packet );

		if ( status != sf::Socket::Done ) {
			if ( status == sf::Socket::Disconnected ) {
				console::log( FMT( "user at ip: %s disconnected \n", session->m_ip.data( ) ) );
			}
			else {
				console::log( FMT( "failed to recieve session type packet from %s \n", session->m_ip.data( ) ) );
			}
			g_session_manager->destroy_session( session );
		}
		if ( !( packet >> session->m_session_type ) ) {
			console::log( FMT( "failed to parse session type from %s \n", session->m_ip.data( ) ) );
			return g_session_manager->destroy_session( session );
		}

		packet.clear( );

		int client_loader_ver = 0;
		const sf::Socket::Status loader_ver_status = session->m_socket.receive(packet);
		if (loader_ver_status != sf::Socket::Done) {
			if (loader_ver_status == sf::Socket::Disconnected) {
				console::log(FMT("failed to get loader version of %s \n", session->m_username.data()));
				return;
			}
		}
		packet >> session->m_loader_version;
		packet.clear();

		switch ( session->m_session_type ) {
		case session_type::LOADER:
			handle_loader_session( session );
			break;
		case session_type::INJECTED:
			handle_injected_session( session );
			break;
		}

		g_session_manager->destroy_session( session );
	}, g_session_manager->get_front( ) );

	return m_thread->joinable( );
}