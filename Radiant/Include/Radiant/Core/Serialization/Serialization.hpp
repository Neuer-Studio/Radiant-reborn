#pragma once

#include <yaml-cpp/yaml.h>
#include <any>

#include <Radiant/Utilities/FileSystem.hpp>

namespace Radiant::Serialization
{
    class Node
    {
    public:
        void AddValue( const std::string& key, const std::any& value );

        void AddChildNode( const std::string& key, const Node& child )
        {
            m_YamlNode[key] = child.m_YamlNode;
        }

        void AddArray( const std::string& key, const std::vector<Node>& array );

        std::string toString() const
        {
            return YAML::Dump( m_YamlNode );
        }

        void saveToFile( const std::string& filename ) const
        {
            std::ofstream fout( filename );
            fout << toString();
            fout.close();
        }

    private:
        YAML::Node m_YamlNode;
    };

    class YamlWriter
    {
    public:
        void AddValue( const std::string& key, const std::any& value )
        {
            m_RootNode.AddValue( key, value );
        }

        void AddNode( const std::string& key, const Node& node )
        {
            m_RootNode.AddChildNode( key, node );
        }

        void AddArray( const std::string& key, const std::vector<Node>& array )
        {
            m_RootNode.AddArray( key, array );
        }

        std::string ToYaml() const
        {
            return m_RootNode.toString();
        }

        void SaveToFile( const std::string& filename ) const
        {
            m_RootNode.saveToFile( filename );
        }

    private:
        Node m_RootNode;
    };

} // namespace Radiant::Serialization

namespace Radiant::Deserialization
{
    class NodeReader
    {
    public:
        // Метод для чтения YAML файла
        bool LoadFromFile( const std::string& filename )
        {
            YAML::Node fileNode = YAML::LoadFile( filename );
            RADIANT_VERIFY( fileNode ); // Проверяем, что файл успешно загружен

            m_YamlNode = fileNode;
            ParseNode( m_YamlNode );
            return true;
        }

        // Получить значение по ключу
        std::any GetValue( const std::string& key ) const
        {
            auto it = m_Values.find( key );
            if ( it != m_Values.end() )
            {
                return it->second;
            }
            return {};
        }

        // Получить дочерние узлы по ключу
        std::vector<NodeReader> GetChildNodes( const std::string& key ) const
        {
            auto it = m_ChildNodes.find( key );
            if ( it != m_ChildNodes.end() )
            {
                return it->second;
            }
            return {};
        }

        // Проверить, существует ли значение по ключу
        bool HasValue( const std::string& key ) const
        {
            return m_Values.find( key ) != m_Values.end();
        }

        // Проверить, существуют ли дочерние узлы по ключу
        bool HasChildNodes( const std::string& key ) const
        {
            return m_ChildNodes.find( key ) != m_ChildNodes.end();
        }

    private:
        YAML::Node                                               m_YamlNode;
        std::unordered_map<std::string, std::any>                m_Values;
        std::unordered_map<std::string, std::vector<NodeReader>> m_ChildNodes;

        // Рекурсивный метод для разбора YAML-узла
        void ParseNode( const YAML::Node& node )
        {
            RADIANT_VERIFY( node.IsDefined() ); // Проверяем, что узел определен

            for ( const auto& it : node )
            {
                std::string key = it.first.as<std::string>();

                if ( it.second.IsScalar() )
                {
                    // Если это одиночное значение (например, Scene: Test Scene)
                    m_Values[key] = it.second.as<std::string>();
                }
                else if ( it.second.IsSequence() )
                {
                    // Если это список сущностей (Entities)
                    std::vector<NodeReader> childNodes;
                    for ( const auto& child : it.second )
                    {
                        NodeReader childNode;
                        childNode.ParseNode( child );
                        childNodes.push_back( childNode );
                    }
                    m_ChildNodes[key] = childNodes;
                }
                else if ( it.second.IsMap() )
                {
                    // Если это объект (например, каждый Entity)
                    NodeReader childNode;
                    childNode.ParseNode( it.second );
                    m_ChildNodes[key].push_back( childNode );
                }
                else
                {
                    RADIANT_VERIFY( false ); // Неожиданный тип данных
                }
            }
        }
    };
} // namespace Radiant::Deserialization