#pragma once

namespace Radiant
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer()
			: Layer("EditorLayer")
		{}

		virtual void OnAttach()
		{
			float* v = new float[6]
			{
				0.0f, 0.5f,
					-0.5f, -0.5f,
					0.5f, -0.5f
			};
			VBO = VertexBuffer::Create((std::byte*)v, 4 * 6);
			sh = Shader::Create("Resources/Shaders/test.radiantshader");
			sh->SetUniform(RadiantShaderType::Fragment, 0, "is_1", true);
		}
		virtual void OnDetach()
		{

		}
		virtual void OnUpdate() override
		{
			VBO->Use();
			sh->Use();
			Rendering::DrawPrimitive(Primitives::Triangle);
		}
	private:
		Memory::Shared<VertexBuffer> VBO;
		Memory::Shared<Shader> sh;
	};
}