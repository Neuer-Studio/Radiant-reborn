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

		auto& GetMaterialDiffuseData()    { return	MaterialDiffuseData; }
		auto& GetMaterialNormalData()     { return	MaterialNormalData; }
		auto& GetMaterialRoughnessData()  {	return  MaterialRoughnessData; }
		auto& GetMaterialMetalnessData()  {	return  MaterialMetalnessData; }

		auto& GetMaterialDiffuseData()	const	{ return	MaterialDiffuseData; }
		auto& GetMaterialNormalData()	const	{ return	MaterialNormalData; }
		auto& GetMaterialRoughnessData()const	{ return  MaterialRoughnessData; }
		auto& GetMaterialMetalnessData()const 	{ return  MaterialMetalnessData; }
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
		struct
		{
			bool Enabled = false;
			Memory::Shared<Texture2D> Texture;
			glm::vec3 AlbedoColor;
		} MaterialDiffuseData;

		struct
		{
			bool Enabled = false;
			Memory::Shared<Texture2D> Texture;
		} MaterialNormalData;

		struct
		{
			bool Enabled = false;
			Memory::Shared<Texture2D> Texture;
			float Roughness;
		} MaterialRoughnessData;

		struct
		{
			bool Enabled = false;
			Memory::Shared<Texture2D> Texture;
			float Metalness;
		} MaterialMetalnessData;
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