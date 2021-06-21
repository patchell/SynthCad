#pragma once

struct RenderBuffer
{
	RenderBuffer *  m_pNext;
	UINT32          m_nBufferLength;
	BYTE *          m_pBuffer;

	RenderBuffer() :
		m_pNext(NULL),
		m_nBufferLength(0),
		m_pBuffer(NULL)
	{
	}

	~RenderBuffer()
	{
		if (m_pBuffer) delete[] m_pBuffer;
	}
};
