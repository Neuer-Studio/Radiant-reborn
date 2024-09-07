#include <Radiant/Core/Memory/Shared.hpp>

namespace Radiant::Memory
{
    static std::unordered_set<void*> s_LiveReferences;
    namespace RefUtils
    {
        bool IsLive( void* instance )
        {
            RADIANT_VERIFY( instance );
            return s_LiveReferences.find( instance ) != s_LiveReferences.end();
        }

        void AddToLiveReferences( void* instance )
        {
            RADIANT_VERIFY( instance );
            s_LiveReferences.insert(instance);
        }

        void RemoveFromLiveReferences( void* instance )
        {
            RADIANT_VERIFY( instance );
            RADIANT_VERIFY( s_LiveReferences.find( instance ) != s_LiveReferences.end() );
            s_LiveReferences.erase( instance );
        }

    } // namespace RefUtils
} // namespace Radiant::Memory