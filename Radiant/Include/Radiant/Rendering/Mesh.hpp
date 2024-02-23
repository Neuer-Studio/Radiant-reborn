#pragma once

#include <Radiant/Rendering/VertexBuffer.hpp>
#include <Radiant/Rendering/IndexBuffer.hpp>

#include <glm/glm.hpp>

namespace Assimp
{
	class Importer;
}

namespace Radiant
{
	struct Vertex {
		glm::vec3 Position;
		glm::vec2 Normals;
		//glm::vec2 TexCoords;
	};

	struct Index
	{
		uint32_t V1, V2, V3;
	};

	enum class TextureType
	{
		None = 0,
		Diffuse,
		Specular, 
		Normal,
	};

	class Mesh : public Memory::RefCounted
	{
	public:
		Mesh(const std::filesystem::path& filepath);

		const std::string& GetName() const { return m_Name; }

		void Use() const;
		uint32_t GetIndexCount() const { return m_IndexBuffer->GetCount(); }
	
		Memory::Shared<VertexBuffer> m_VertexBuffer;
		Memory::Shared<IndexBuffer> m_IndexBuffer;

		std::vector<Vertex> m_Vertices;
		std::vector<Index> m_Indices;

		std::string m_Name;
		std::filesystem::path m_AssetPath;
	};

	class StaticMesh : public Mesh
	{
	public:
	};
	
	class AnimatedMesh : public Mesh
	{
	public:
	};
}