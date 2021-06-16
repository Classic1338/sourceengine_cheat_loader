#pragma once
#include <memory>
#include <deque>
#include <thread>
#include <Windows.h>
#include <SFML/Network.hpp>
#include <sstream>
#include <iostream>

#include "../session/session.h"
#include "../dependencies/json/json.hpp"

static size_t write_callback( char* contents, size_t size, size_t nmemb, void* userp ) {
	( ( std::string* ) userp )->append( ( char* ) contents, size * nmemb );
	return size * nmemb;
}

struct get_login_info {
	get_login_info( std::string a_username, std::string a_password, std::string a_api_key ) :
	m_username( a_username ),
	m_password( a_password ),
	m_api_key( a_api_key )
		{ }

	bool is_user( ) {
		bool valid_user = false;

		sf::Http http;
		http.setHost( "http://www.saturn.technology" );

		sf::Http::Request request;
		request.setMethod( sf::Http::Request::Post );
		request.setUri( "api/auth" );

		request.setField( "XF-Api-Key", "kTaXHHJM6YnKAhXwCmjTdUqSkoZN0vKz" );
		request.setField( "XF-Api-User", "1" );
		request.setField( "api_bypass_permissions", "1" );

		std::string data = "login=" + m_username + "&password=" + m_password;
		request.setBody(data);

		sf::Http::Response response = http.sendRequest( request );

	//	std::cout << "body: " << response.getBody( ) << std::endl;

		if (response.getStatus( ) != sf::Http::Response::Ok) {
			printf( "response failed \n" );
			return false;
		}

		auto json = nlohmann::json::parse( response.getBody( ) );
		if (!json.contains( "user" )) {
			printf( "user %s is an invalid user \n", m_username.c_str( ) );
			return false;
		}

		auto user = json [ "user" ];

		if (!user.contains( "is_banned" )) {
			printf(
				"json response for user %s doesn't contain is_banned. \n", m_username.c_str() );
			return false;
		}
		m_user_banned = user["is_banned"].get<bool>();

		if (!user.contains( "user_id" )) {
			printf(
				"json response for user %s doesn't contain user_id. \n", m_username.c_str());
			return false;
		}
		m_uid = user [ "user_id" ].get<int>( );

		for (int i = 0; i < user["secondary_group_ids"].size(); i++) {
			auto value = user["secondary_group_ids"].at(i);
			if (value == 6)
				m_user_beta = true;
			if (value == 5)
				m_user_premium = true;
		}

		return true;
	}

	int m_uid = 0;
	bool m_user_banned = false, m_user_beta = false, m_user_premium = false;
	std::string m_username, m_password, m_api_key;
};

class session_manager {
public:
	std::shared_ptr< session > create_session( );
	void destroy_session( std::shared_ptr< session > session );
	std::shared_ptr<session> get_front( );

private:
	std::deque<std::shared_ptr<session>> session_data;
};
extern std::unique_ptr< session_manager > g_session_manager;