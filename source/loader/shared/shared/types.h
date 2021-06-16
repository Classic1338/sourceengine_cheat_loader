#pragma once
#include <cstdint>
#include <SFML/Network.hpp>

enum class session_state : uint8_t {
	NONE,
	AUTHENTICATED,
	INJECTED
};

enum class session_type : uint8_t {
	LOADER,
	INJECTED
};

enum class game_type : uint8_t {
	NONE,
	CSGO,
	L4D2
};

static sf::Packet& operator <<( sf::Packet& packet, const session_type& type ) {
	return packet << static_cast< uint8_t >( type );
}

static sf::Packet& operator >>( sf::Packet& packet, session_type& type ) {
	return packet >> *reinterpret_cast< uint8_t* >( &type );
}

static sf::Packet& operator <<( sf::Packet& a_packet, const std::vector<char>& a_vec ) {
    a_packet << a_vec.size( );
    for ( const char& c : a_vec ) {
        a_packet << static_cast< sf::Int8 >( c );
    }

    return a_packet;
}

static sf::Packet& operator >>( sf::Packet& a_packet, std::vector<char>& a_vec ) {
    a_vec.clear( );
    size_t size;
    a_packet >> size;
    a_vec.reserve( size );
    a_vec.resize( size );

    for ( unsigned int i = 0; i < size; i++ ) {
        a_packet >> *reinterpret_cast< sf::Int8* >( &a_vec [ i ] );
    }

    return a_packet;
}