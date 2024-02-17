#include <Radiant/Rendering/Mesh.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

namespace Radiant
{
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

	Mesh::Mesh(const std::filesystem::path& filepath)
	{
		RADIANT_VERIFY(Utils::FileSystem::Exists(filepath));
		RA_INFO("Loading mesh: {0}", filepath.string().c_str());

		m_Name = Utils::FileSystem::GetFileName(filepath);

		LogStream::Initialize();

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath.string(), aiProcess_Triangulate | aiProcess_FlipUVs);

		aiMesh * mesh = scene->mMeshes[0];

		m_Vertices.reserve(mesh->mNumVertices);
		m_Indices.reserve(mesh->mNumFaces);

		for (int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.Normals = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

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
	}

	void Mesh::Use() const
	{
		m_VertexBuffer->Use();
		m_IndexBuffer->Use();
	}

}