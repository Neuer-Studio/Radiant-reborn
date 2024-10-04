#pragma once

#include <optional>
#include <variant>
#include <string>

namespace Common
{
    template <typename T>
    class Result;

    template <typename T>
    Result<T> MakeError( const std::string& message );

    template <typename T>
    Result<T> MakeSuccess( const T& value );

    template <typename T>
    class Result
    {
    public:
        class Error
        {
        public:
            explicit Error( const std::string& errorMessage ) : m_ErrorMessage( errorMessage )
            {
            }

            const std::string& GetMessage() const
            {
                return m_ErrorMessage;
            }

        private:
            std::string m_ErrorMessage;
        };

        bool IsSuccess() const
        {
            return m_IsSuccess;
        }

        const std::optional<T> GetValue() const
        {
            if ( !m_IsSuccess )
            {
                return std::nullopt;
            }
            return std::get<T>( m_Outcome );
        }

        std::string GetError() const
        {
            if ( m_IsSuccess )
            {
                return s_NoError;
            }
            return std::get<Error>( m_Outcome ).GetMessage();
        }

    private:
        explicit Result( const Error& error ) : m_Outcome( error ), m_IsSuccess( false )
        {
        }

        explicit Result( const T& value ) : m_Outcome( value ), m_IsSuccess( true )
        {
        }

        static inline std::string s_NoError = "Cannot get error message, result is a success";
        std::variant<T, Error>            m_Outcome;
        bool                              m_IsSuccess;

    private:
        friend Result<T> MakeError<T>( const std::string& message );
        friend Result<T> MakeSuccess<T>( const T& value );
    };

    template <typename T>
    Result<T> MakeError( const std::string& message )
    {
        return Result<T>( Result<T>::Error( message ) );
    }

    template <typename T>
    Result<T> MakeSuccess( const T& value )
    {
        return Result<T>( value );
    }

} // namespace Common
