#include <memory>
#include <deque>
#include <thread>
#include <SFML/Network.hpp>
#include <shared/packet.h>
#include <shared/string_utils.h>
#include <shared/console.h>
#include <shared/types.h>

struct session {
	bool start( );

	std::shared_ptr<std::thread> m_thread;
	session_state m_session_state;
	session_type m_session_type;
	sf::TcpSocket m_socket;
	std::string	m_ip;
	bool m_terminated;
	std::string m_username;
	int m_game_type;
	int m_cheat_selection;
	uint64_t m_key_timestamp, m_key;
	int m_loader_version;
};