#pragma once

#include <stdint.h>
#include <map>
#include <string>
#include <typeinfo>
#include <mutex>
#include <memory>
#include <utility>
#include <iostream>

namespace Radiant::Memory
{

    namespace RefUtils
    {
        bool IsLive( void* instance );
        void AddToLiveReferences( void* instance );
        void RemoveFromLiveReferences( void* instance );
    } // namespace RefUtils

#if defined( RADIANT_CONFIG_DEBUG )
    inline uint32_t                        s_TotalCreated   = 0;
    inline uint32_t                        s_TotalDestroyed = 0;
    inline std::map<std::string, uint32_t> s_ObjectMap;
    inline std::mutex                      s_MapMutex;

    inline void RegisterObjectCreation( const std::string& typeName )
    {
        std::lock_guard<std::mutex> lock( s_MapMutex );
        s_ObjectMap[typeName]++;
        s_TotalCreated++;
    }

    inline void RegisterObjectDestruction( const std::string& typeName )
    {
        std::lock_guard<std::mutex> lock( s_MapMutex );
        auto                        it = s_ObjectMap.find( typeName );
        if ( it != s_ObjectMap.end() )
        {
            if ( --( it->second ) == 0 )
            {
                s_ObjectMap.erase( it );
            }
        }
        s_TotalDestroyed++;
    }

#endif

    class RefCounted
    {
    public:
        template <typename T>
        void IncRefCount( T t ) const
        {
#if defined( RADIANT_CONFIG_DEBUG )
            RegisterObjectCreation( typeid( t ).name() );
#endif
            m_RefCount++;
        }

        void DecRefCount() const
        {
#if defined( RADIANT_CONFIG_DEBUG )
            RegisterObjectDestruction( typeid( *this ).name() );
#endif
            m_RefCount--;
        }

        uint32_t GetRefCount() const
        {
            return m_RefCount;
        }

    private:
        mutable uint32_t m_RefCount = 0; // TODO: atomic
    };

    template <typename T>
    class Shared
    {
    public:
        Shared() : m_Instance( nullptr )
        {
        }

        Shared( std::nullptr_t n ) : m_Instance( nullptr )
        {
        }

        Shared( T* instance ) : m_Instance( instance )
        {
            static_assert( std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!" );

            IncRef();
        }

        template <typename T2>
        Shared( const Shared<T2>& other )
        {
            m_Instance = static_cast<T*>( other.m_Instance );
            IncRef();
        }

        template <typename T2>
        Shared( Shared<T2>&& other ) noexcept
        {
            m_Instance       = static_cast<T*>( other.m_Instance );
            other.m_Instance = nullptr;
        }

        ~Shared()
        {
            DecRef();
        }

        Shared( const Shared<T>& other ) : m_Instance( other.m_Instance )
        {
            IncRef();
        }

        Shared& operator=( std::nullptr_t )
        {
            DecRef();
            m_Instance = nullptr;
            return *this;
        }

        Shared& operator=( const Shared<T>& other )
        {
            other.IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            return *this;
        }

        template <typename T2>
        Shared& operator=( const Shared<T2>& other )
        {
            other.IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            return *this;
        }

        template <typename T2>
        Shared& operator=( Shared<T2>&& other ) noexcept
        {
            DecRef();

            m_Instance       = other.m_Instance;
            other.m_Instance = nullptr;
            return *this;
        }

        operator bool() const
        {
            return m_Instance != nullptr;
        }

        T* operator->()
        {
            return m_Instance;
        }

        const T* operator->() const
        {
            return m_Instance;
        }

        T& operator*()
        {
            return *m_Instance;
        }

        const T& operator*() const
        {
            return *m_Instance;
        }

        T* Raw()
        {
            return m_Instance;
        }

        const T* Raw() const
        {
            return m_Instance;
        }

#if defined( RADIANT_CONFIG_DEBUG )
        inline static uint32_t DEBUG_GetTotalDestroyed()
        {
            return s_TotalDestroyed;
        }

        inline static uint32_t DEBUG_GetTotalCreated()
        {
            return s_TotalCreated;
        }

        inline static auto DEBUG_GetLiveObjects()
        {
            return s_ObjectMap;
        }
#endif

        void Reset( T* instance = nullptr )
        {
            DecRef();
            m_Instance = instance;
        }

        template <typename T2>
        Shared<T2> As() const
        {
            return Shared<T2>( *this );
        }

        template <typename... Args>
        static Shared<T> Create( Args&&... args )
        {
            return Shared<T>( new T( std::forward<Args>( args )... ) );
        }

    private:
        void IncRef() const
        {
            if ( m_Instance )
                m_Instance->IncRefCount( m_Instance );
        }

        void DecRef() const
        {
            if ( m_Instance )
            {
                m_Instance->DecRefCount();
                if ( m_Instance->GetRefCount() == 0 )
                {
                    delete m_Instance;
                }
            }
        }

        template <class T2>
        friend class Shared;
        T* m_Instance;
    };

    template <typename T>
    class Weak
    {
    public:
        Weak() = default;

        Weak( T* instance ) : m_Instance( instance )
        {
        }

        Weak( Shared<T> object ) : m_Instance( object.Raw() )
        {
        }

        T* operator->()
        {
            return m_Instance;
        }

        const T* operator->() const
        {
            return m_Instance;
        }

        T& operator*()
        {
            return *m_Instance;
        }

        const T& operator*() const
        {
            return *m_Instance;
        }

        void Reset()
        {
            m_Instance = nullptr;
        }

        bool IsValid() const
        {
            return m_Instance ? RefUtils::IsLive( m_Instance ) : false;
        }

        const T* Raw() const
        {
            return m_Instance;
        }

        T* Raw()
        {
            return m_Instance;
        }

    private:
        T* m_Instance = nullptr;
    };

} // namespace Radiant::Memory
