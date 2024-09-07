#include <Radiant/Core/Serialization/Serialization.hpp>

namespace Radiant::Serialization
{

    void Node::AddValue( const std::string& key, const std::any& value )
    {
        if ( value.type() == typeid( int ) )
        {
            m_YamlNode[key] = std::any_cast<int>( value );
        }
        else if ( value.type() == typeid( float ) )
        {
            m_YamlNode[key] = std::any_cast<float>( value );
        }
        else if ( value.type() == typeid( double ) )
        {
            m_YamlNode[key] = std::any_cast<double>( value );
        }
        else if ( value.type() == typeid( std::string ) )
        {
            m_YamlNode[key] = std::any_cast<std::string>( value );
        }
        else if ( value.type() == typeid( std::vector<int> ) )
        {
            m_YamlNode[key] = YAML::Node( YAML::NodeType::Sequence );
            for ( const auto& item : std::any_cast<std::vector<int>>( value ) )
            {
                m_YamlNode[key].push_back( item );
            }
        }
        else
        {
            RA_ERROR( "Unsupported value type {}", key );
        }
    }

    void Node::AddArray( const std::string& key, const std::vector<Node>& array )
    {
        YAML::Node yamlArray = YAML::Node( YAML::NodeType::Sequence );
        for ( const auto& node : array )
        {
            yamlArray.push_back( node.m_YamlNode );
        }
        m_YamlNode[key] = yamlArray;
    }
} // namespace Radiant::Serialization