#include "../pch.hpp"
#include "QuadMesh.hpp"

namespace Bolt {

	//struct PosTexVertexLocal { float x, y, z; float u, v; };


	//static const PosTexVertexLocal s_UnitQuad[4] = {
	//{ -0.5f, 0.5f, 0.0f, 0.0f, 0.0f }, 
	//{ -0.5f, -0.5f, 0.0f, 0.0f, 1.0f }, 
	//{ 0.5f, -0.5f, 0.0f, 1.0f, 1.0f }, 
	//{ 0.5f, 0.5f, 0.0f, 1.0f, 0.0f }, 
	//};


	//static const uint16_t s_Indices[6] = { 0,1,3, 3,1,2 };


	//bool QuadMesh::Init(const bgfx::VertexLayout& layout) {
	//	if (bgfx::isValid(m_VB) || bgfx::isValid(m_IB)) return true;


	//	m_VB = bgfx::createVertexBuffer(
	//		bgfx::makeRef(s_UnitQuad, sizeof(s_UnitQuad)),
	//		layout
	//	);
	//	if (!bgfx::isValid(m_VB)) {
	//		Logger::Error( "[QuadMesh] Failed to create vertex buffer\n");
	//		return false;
	//	}


	//	m_IB = bgfx::createIndexBuffer(bgfx::makeRef(s_Indices, sizeof(s_Indices)));
	//	if (!bgfx::isValid(m_IB)) {
	//		Logger::Error("[QuadMesh] Failed to create index buffer\n");
	//		bgfx::destroy(m_VB);
	//		m_VB = BGFX_INVALID_HANDLE;
	//		return false;
	//	}


	//	return true;
	//}


	//void QuadMesh::Shutdown() {
	//	if (bgfx::isValid(m_VB)) { bgfx::destroy(m_VB); m_VB = BGFX_INVALID_HANDLE; }
	//	if (bgfx::isValid(m_IB)) { bgfx::destroy(m_IB); m_IB = BGFX_INVALID_HANDLE; }
	//}
}