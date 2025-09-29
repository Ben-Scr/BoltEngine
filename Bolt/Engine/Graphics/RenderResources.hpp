#pragma once


namespace Bolt {

	/*struct VertexPosTexColor {
		float x, y, z;
		float u, v;
		uint32_t abgr;


		static void SetLayout(bgfx::VertexLayout& layout) {
			layout.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, false)
				.end();
		}
	};


	class RenderResources {
	public:
		RenderResources() = default;
		~RenderResources() = default;

		bool Init();
		void Shutdown();


		const bgfx::VertexLayout& GetPosTexLayout() const { return m_PosTexLayout; }
		const bgfx::VertexLayout& GetPosColorLayout() const { return m_PosColorLayout; }


		bgfx::UniformHandle GetSamplerUniform() const { return m_SamplerUniform; }
		bgfx::TextureHandle GetDefaultTexture() const { return m_DefaultTexture; }

	private:
		bgfx::VertexLayout m_PosTexLayout;
		bgfx::VertexLayout m_PosColorLayout;

		bgfx::UniformHandle m_SamplerUniform = BGFX_INVALID_HANDLE;
		bgfx::TextureHandle m_DefaultTexture = BGFX_INVALID_HANDLE;
	};
*/

}