#include <Radiant/Rendering/Mesh.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

namespace Radiant
{

#define MESH_DEBUG_LOG 1
#if MESH_DEBUG_LOG
#define MESH_LOG(...) RA_INFO(__VA_ARGS__)
#else
#define MESH_LOG(...)
#endif

	struct LogStream : public Assimp::LogStream
	{
		static void Initialize()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
				Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
			}
		}

		void write(const char* message) override
		{
			RA_ERROR("Assimp error: {0}", message);
		}
	};

	constexpr unsigned int ImportFlags =
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_SortByPType |
		aiProcess_PreTransformVertices |
		aiProcess_GenNormals |
		aiProcess_GenUVCoords |
		aiProcess_OptimizeMeshes |
		aiProcess_Debone |
		aiProcess_ValidateDataStructure;

	Mesh::Mesh(const std::filesystem::path& filepath)
	{
		RADIANT_VERIFY(Utils::FileSystem::Exists(filepath));
		RA_INFO("Loading mesh: {0}", filepath.string().c_str());
		RA_INFO("id: {}", typeid(MaterialDiffuseData).name());

		m_Name = Utils::FileSystem::GetFileName(filepath);

		LogStream::Initialize();

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath.string(), ImportFlags);

		aiMesh * mesh = scene->mMeshes[0];

		m_Vertices.reserve(mesh->mNumVertices);
		m_Indices.reserve(mesh->mNumFaces);

		for (int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.Normals = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

			if (mesh->HasTangentsAndBitangents()) {
				vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
				vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			}
			if (mesh->HasTextureCoords(0))
				vertex.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};

			m_Vertices.push_back(vertex);
		}

		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));

		for (int i = 0; i < mesh->mNumFaces; i++)
		{
			RADIANT_VERIFY(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices.");
			Index index;
			index = {mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2]};

			m_Indices.push_back(index);
		}

		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size() * sizeof(Index));

		RADIANT_VERIFY(scene->HasMaterials());
		if (scene->HasMaterials())
		{ 
			MESH_LOG("=====================================", filepath.string());
			MESH_LOG("====== Materials - {0} ======", filepath.string());
			MESH_LOG("=====================================", filepath.string());

			for (unsigned int i = 0; i < scene->mNumMaterials; i++)
			{
				const aiMaterial* material = scene->mMaterials[i];
				aiString texturePath;
				aiColor3D aiColor;
				
				MaterialDiffuseData.AlbedoColor = { 0.0, 0.0,0.0 };

				if (material->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor) == aiReturn_SUCCESS)
					MaterialDiffuseData.AlbedoColor = { aiColor.r, aiColor.g, aiColor.b };

				if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)//TODO: is texture loaded -> set a texture to material, or just set a vec3 data
				{
					std::filesystem::path imagePath = Utils::FileSystem::GetFileDirectory(filepath) / std::filesystem::path(texturePath.C_Str());

					MaterialDiffuseData.Enabled = true;
					MaterialDiffuseData.Texture = Texture2D::Create(imagePath);

					MESH_LOG("aiTextureType_DIFFUSE: {}", imagePath.string());
				}

				if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
				{
					std::filesystem::path imagePath = Utils::FileSystem::GetFileDirectory(filepath) / std::filesystem::path(texturePath.C_Str());

					MaterialNormalData.Enabled = true;
					MaterialNormalData.Texture = Texture2D::Create(imagePath);

					MESH_LOG("aiTextureType_NORMALS: {}", imagePath.string());
				}

				MaterialRoughnessData.Roughness = 1.0f;
				if (material->GetTexture(aiTextureType_SHININESS, 0, &texturePath) == AI_SUCCESS)
				{
					std::filesystem::path imagePath = Utils::FileSystem::GetFileDirectory(filepath) / std::filesystem::path(texturePath.C_Str());

					MaterialRoughnessData.Enabled = true;
					MaterialRoughnessData.Texture = Texture2D::Create(imagePath);

					MESH_LOG("aiTextureType_SHININESS: {}", imagePath.string());
				}

				MaterialMetalnessData.Metalness = 0.5f;
				if (material->Get("$raw.ReflectionFactor|file", aiPTI_String, 0, texturePath) == AI_SUCCESS)
				{
					std::filesystem::path imagePath = Utils::FileSystem::GetFileDirectory(filepath) / std::filesystem::path(texturePath.C_Str());

					MaterialMetalnessData.Enabled = true;
					MaterialMetalnessData.Texture = Texture2D::Create(imagePath);

					MESH_LOG("aiTextureType_SHININESS: {}", imagePath.string());
				}
			}
		}
	}

	void Mesh::Use() const
	{
		m_VertexBuffer->Use();
		m_IndexBuffer->Use();
	}

}