#include <Radiant/Scene/Entity.hpp>

#include <Radiant/Scene/Scene.hpp>

namespace Radiant
{
    Entity::~Entity()
    {
    }

    void Entity::SetParent( Entity parent )
    {
        auto& currentParent = GetParent();
        if ( currentParent && parent == *currentParent )
        {
            return;
        }

        if ( currentParent )
            ( *currentParent ).RemoveChild( *this );

        SetParentUUID( parent.GetUUID() );

        if ( parent )
        {
            auto& parentChildren = parent.Children();
            UUID  uuid           = GetUUID();
            if ( std::find( parentChildren.begin(), parentChildren.end(), uuid ) == parentChildren.end() )
                parentChildren.emplace_back( GetUUID() );
        }
    }

} // namespace Radiant