#include <iostream>
#include <Windows.h>
#include <conio.h>

#include <SFML/Network.hpp>

#include <shared/console.h>
#include <shared/constants.h>
#include <shared/string_utils.h>
#include <shared/types.h>
#include <shared/packet.h>

#include <BlackBone/Process/Process.h>

#include "encrypt_string.h"

using namespace blackbone;
#pragma comment (lib, "blackbone.lib")

namespace globals {
	std::string username;
}

std::string get_username( ) {
	std::string username;
	std::cout << str("username: ");
	std::cin >> username;
	return username;
}

std::string hide_password() {
	std::string password;
	std::cout << str("password: ");
	char c;
	while ((c = _getch()) != 13) {
		if (c == 8) {
			if (password.empty()) {
				continue;
			}

			password.pop_back();
			printf("\b \b");
			continue;
		}

		password += c;
		printf(str("*"));
	}

	printf("\n");

	return password;
}

void admin_panel() {
	printf("admin panel: \n");
}

void inject_to_process( std::wstring string, int game_selection, sf::TcpSocket& socket ) {
	packet_t packet;
	Process game_process;
	std::vector<DWORD> processes;
	do {
		processes = game_process.EnumByName(string.data());
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	} while ( processes.empty( ) );
	if (NT_ERROR(game_process.Attach(processes.at(0)))) {
		console::log(FMT(str("failed to attach to process %s \n"), string.data()));
	}
	while (!game_process.modules().GetModule(str(L"serverbrowser.dll"))) {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	packet.clear();
	socket.receive(packet);
	uint64_t key;
	std::vector<char> module_byte;
	packet >> key >> module_byte;

	auto inject = game_process.mmap().MapImage(module_byte.size(), module_byte.data());

	if (!inject) {
		std::wcout << str(L"Mapping failed with error 0x") << std::hex << inject.status 
			<< str(L". ") << Utils::GetErrorDescription(inject.status) << std::endl << std::endl;
	}
	else
		console::log(FMT(str("enjoy! welcome to saturn! %s \n"), globals::username));

	Sleep(INFINITE);
}

int main() {
	sf::TcpSocket socket;
	if (socket.connect(str(NETWORK_IP), NETWORK_PORT, sf::seconds(2)) != sf::Socket::Done) {
		console::log(str("failed to connect to server \n"));
		Sleep(2000);
		return EXIT_FAILURE;
	}

	printf(str("Welcome to saturn! \n"));
pre_login:

	std::string username = get_username();
	std::string password = hide_password();

	packet_t packet;

	packet << session_type::LOADER;
	if (socket.send(packet) != sf::Socket::Done)
		console::log(str("socket failed to send \n"));
	packet.clear();

	packet << LOADER_VERSION;
	if (socket.send(packet) != sf::Socket::Done)
		console::log(str("socket failed to send \n"));
	packet.clear();

	packet << username;
	packet << password;
	if ( socket.send( packet ) != sf::Socket::Done )
		console::log(str("socket failed to send \n") );

	packet.clear( );

	socket.receive( packet );

	std::string successful;
	packet >> successful;

	bool logged_in, second_logged_in;
	packet >> logged_in;

	second_logged_in = logged_in;

	if (logged_in)
		console::log(FMT(str("%s \n"), successful.data()));

	if (!second_logged_in) {
		console::log(successful.c_str());
		Sleep(1000);
		exit(0);
	}

	if (strcmp(successful.c_str(), str("logged in")) && !logged_in) {
		printf(str("user not premium"));
		exit(0);
	}

	bool is_beta = false, is_banned = false, is_admin = false;
	packet >> is_beta;
	packet >> is_banned;

	if (is_banned) {
		Sleep(1000);
		exit(0);
	}

	while ( !FindWindowA(str("Valve001"), NULL ) ) {
		static bool call_once = false;
		if ( !call_once ) {
			console::log(str("waiting for a game \n") );
			call_once = true;
		}
		Sleep( 200 );
	}

	packet.clear( );
	int game_selection = 1;

	if ( is_beta ) {
select_game:
		console::log(str("please enter the cheat you want to inject: \n") );
		console::log(str("1: csgo \n"));
		console::log(str("2: l4d2 \n"));
		std::cin >> game_selection;
		if ( game_selection > 2 || game_selection < 1 ) {
			console::log(str("error: invalid selection \n"));
			goto select_game;
		}
	}

	packet << game_selection;
	if ( socket.send( packet ) != sf::Socket::Done )
		console::log(str("failed to send cheat selection \n"));
	packet.clear( );

	globals::username = username;

	int cheat_selection;
select_cheat:

	console::log(str("please enter the cheat you want to use: \n"));
	console::log(str("1: main build \n"));
	if (is_beta)
		console::log(str("2: beta build \n"));
	std::cin >> cheat_selection;

	if (cheat_selection > 2 || cheat_selection < 1) {
		console::log(str("error: invalid selection \n"));
		goto select_cheat;
	}

	packet << cheat_selection;

	if ( socket.send( packet ) != sf::Socket::Done )
		console::log(str("failed to send cheat selection \n"));

	std::wstring game_name;
	switch ( game_selection ) {
	case 0:
		break;
	case 1:
		game_name = str(L"csgo.exe");
		break;
	case 2:
		game_name = str(L"left4dead2.exe");
		break;
	}

	packet.clear( );
	inject_to_process( game_name, game_selection, socket );
}