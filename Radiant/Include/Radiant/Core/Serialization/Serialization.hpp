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

        std::string ToString() const
        {
            return YAML::Dump( m_YamlNode );
        }

        void SaveToFile( const std::string& filename ) const
        {
            Radiant::Utils::FileSystem::WriteContentToFile(filename, ToString());
            std::ofstream fout( filename );
            fout << ToString();
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
            return m_RootNode.ToString();
        }

        void SaveToFile( const std::string& filename ) const
        {
            m_RootNode.SaveToFile( filename );
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
        NodeReader() = default;
        explicit NodeReader(const serialized_str& context);
        bool LoadFromFile(const std::string& filename);

        std::optional<std::any> GetValue(const std::string& key) const;

        std::optional<std::vector<NodeReader>> GetChildNodes(const std::string& key) const;

        bool HasValue(const std::string& key) const;
        bool HasChildNodes(const std::string& key) const;

    private:
        YAML::Node                                               m_YamlNode;
        std::unordered_map<std::string, std::any>                m_Values;
        std::unordered_map<std::string, std::vector<NodeReader>> m_ChildNodes;

        void ParseNode(const YAML::Node& node);
        void MakeContext(const YAML::Node& node);
    };
} // namespace Radiant::Deserialization