#pragma once

namespace Radiant
{
    class UUID
    {
    public:
        UUID();
        explicit UUID( uint64_t uuid );
        UUID( const UUID& other );

        const std::string ToString() const
        {
            return std::to_string( m_UUID );
        }

        operator uint64_t()
        {
            return m_UUID;
        }
        operator const uint64_t() const
        {
            return m_UUID;
        }

    private:
        uint64_t m_UUID;
    };
} // namespace Radiant

namespace std
{

    template <>
    struct hash<Radiant::UUID>
    {
        std::size_t operator()( const Radiant::UUID& uuid ) const
        {
            // uuid is already a randomly generated number, and is suitable as a hash key as-is.
            // this may change in future, in which case return hash<uint64_t>{}(uuid); might be more
            // appropriate
            return uuid;
        }
    };
} // namespace std