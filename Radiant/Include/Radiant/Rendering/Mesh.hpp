#pragma once

#include <Radiant/Rendering/VertexBuffer.hpp>
#include <Radiant/Rendering/IndexBuffer.hpp>
#include <Radiant/Rendering/Material.hpp>

#include <glm/glm.hpp>

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace Assimp
{
	class Importer;
}

namespace Radiant
{
	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normals;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;
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

	struct Submesh
	{
		uint32_t BaseVertex;
		uint32_t BaseIndex;
		uint32_t MaterialIndex;
		uint32_t IndexCount;

		glm::mat4 Transform;
	};

	class Mesh : public Memory::RefCounted
	{
	public:
		Mesh(const std::filesystem::path& filepath);

		std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
		const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }

		const std::string& GetName() const { return m_Name; }

		void Use() const;
		uint32_t GetIndexCount() const { return m_IndexBuffer->GetCount(); }

		const auto& GetVertexBuffer() const { return m_VertexBuffer; }
		const auto& GetIndexBuffer() const { return m_IndexBuffer; }
	private:
		void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);
	private:
		std::vector<Submesh> m_Submeshes;

		Memory::Shared<VertexBuffer> m_VertexBuffer;
		Memory::Shared<IndexBuffer> m_IndexBuffer;
		Memory::Shared<Material> m_Material;

		std::vector<Vertex> m_StaticVertices;
		std::vector<Index> m_Indices;

		std::string m_Name;
		std::filesystem::path m_AssetPath;

		//Note: Enabled - flag: is texture has been loaded

		struct BaseMeshMaterial
		{
			bool Enabled = false;
			Memory::Shared<Texture2D> Texture;
		};

		struct
		{
			BaseMeshMaterial Material;
			glm::vec3 AlbedoColor;
		} MaterialDiffuseData;

		struct
		{
			BaseMeshMaterial Material;
		} MaterialNormalData;

		struct
		{
			BaseMeshMaterial Material;
			float Roughness;
		} MaterialRoughnessData;

		struct
		{
			BaseMeshMaterial Material;
			float Metalness;
		} MaterialMetalnessData;
	private:
		friend class Rendering;
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