#pragma once
#include <string>
#include <string_view>
#include <type_traits>

#define FMT( str, ...) n_string_utils::format_string( str, __VA_ARGS__ )
#define HASH( str ) n_string_utils::hash_string( str )
#define HASH_CONST( str ) n_string_utils::hash_string_const( str )

namespace n_string_utils {
    template<typename ... args>
    std::string format_string( std::string_view a_format, args ... a_args ) {
        const size_t size = std::snprintf( nullptr, 0, a_format.data( ), a_args ... ) + 1;
        if ( size <= 0 ) {
            throw std::runtime_error( "failed to format string" );
        }

        std::unique_ptr<char[ ]> buf( new char [ size ] );
        std::snprintf( buf.get( ), size, a_format.data( ), a_args ... );

        return std::string( buf.get( ), buf.get( ) + size - 1 );
    }

    static constexpr uint32_t m_basis = 0x811C9DC5;
    static constexpr uint32_t m_prime = 0x1000193;

    constexpr uint32_t hash_string_const( const char* a_string, const uint32_t a_value = m_basis ) noexcept {
        return !*a_string ? a_value : hash_string_const( &a_string [ 1 ], ( a_value ^ uint32_t( a_string [ 0 ] ) ) * m_prime );
    }

    static uint32_t hash_string( const char* a_string ) {
        uint32_t hashed = m_basis;

        for ( size_t i = 0U; i < strlen( a_string ); ++i ) {
            hashed ^= a_string [ i ];
            hashed *= m_prime;
        }

        return hashed;
    }
}