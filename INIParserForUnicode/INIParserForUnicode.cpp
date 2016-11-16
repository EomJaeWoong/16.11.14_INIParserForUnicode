#include <stdio.h>
#include <Windows.h>
#include "INIParserForUnicode.h"

/*------------------------------------------------------------------------------*/
//������
/*------------------------------------------------------------------------------*/
CAyaINIParserUni::CAyaINIParserUni()
{
	m_wpBuffer = new WCHAR[eBUFFER_SIZE];
	Initial();
}

/*------------------------------------------------------------------------------*/
// �ı���
/*------------------------------------------------------------------------------*/
CAyaINIParserUni::~CAyaINIParserUni()
{
	delete[] m_wpBuffer;
}

/*------------------------------------------------------------------------------*/
// �ʱ�ȭ
/*------------------------------------------------------------------------------*/
void CAyaINIParserUni::Initial()
{
	memset(m_wpBuffer, 0, eBUFFER_SIZE);
	m_iLoadSize = 0;

	m_iBufferAreaStart = -1;
	m_iBufferAreaEnd = -1;
	m_iBufferFocusPos = -1;

	m_bProvideAreaMode = FALSE;
}

/*------------------------------------------------------------------------------*/
// ���� �ε�
/*------------------------------------------------------------------------------*/
BOOL CAyaINIParserUni::LoadFile(WCHAR *szFileName)
{
	FILE *fp;
	DWORD dReadSize;

	//----------------------------------------------------------------------------
	// ���� ���� �� ���� �˻�
	//----------------------------------------------------------------------------
	_wfopen_s(&fp, szFileName, L"r,ccs=UTF-8");
	if (fp == NULL)
	{
		wprintf(L"Don't exist file...\n");
		return FALSE;
	}

	//----------------------------------------------------------------------------
	// BOM �ڵ� ����
	//----------------------------------------------------------------------------
	rewind(fp); 

	//----------------------------------------------------------------------------
	// ���� ������ ����
	//----------------------------------------------------------------------------
	fseek(fp, 0, SEEK_END);
	m_iLoadSize = ftell(fp);
	rewind(fp);

	//----------------------------------------------------------------------------
	// ���� ����� ���ۺ��� Ŭ ��
	//----------------------------------------------------------------------------
	if (m_iLoadSize > eBUFFER_SIZE)
	{
		wprintf(L"File is too large...\n");
		fclose(fp);
		Initial();
		return FALSE;
	}

	//----------------------------------------------------------------------------
	// ���� ���� ���� �б�
	//----------------------------------------------------------------------------
	fread_s(m_wpBuffer, eBUFFER_SIZE, m_iLoadSize, 1, fp);

	if (dReadSize != m_iLoadSize)
	{
		wprintf(L"LoadSize not equal BufferSize\n");
		fclose(fp);
		Initial();
		return FALSE;
	}

	fclose(fp);

	return TRUE;
}

/*------------------------------------------------------------------------------*/
// ���� ã��
/*------------------------------------------------------------------------------*/
BOOL CAyaINIParserUni::ProvideArea(WCHAR *szAreaName)
{
	WCHAR *wpTitleBuffer, *wpSpaceBuffer;
	int iLength;
	BOOL bOpenArea = FALSE;

	m_bProvideAreaMode = TRUE;

	while (GetNextWord(&wpTitleBuffer, &iLength))
	{
		//------------------------------------------------------------------------
		// ���� �ñ׳� ã����
		//------------------------------------------------------------------------
		if (wpTitleBuffer[0] == L':')
		{ 
			//--------------------------------------------------------------------
			// ���� �̸� ã��
			//--------------------------------------------------------------------
			if (0 == wcscmp(++wpTitleBuffer, szAreaName))
			{
				if (GetNextWord(&wpSpaceBuffer, &iLength))
				{
					//------------------------------------------------------------
					// '{' ���� �˻�
					//------------------------------------------------------------
					if (wpSpaceBuffer[0] == L'{')
					{
						if (!SkipNoneCommand())
						{
							m_bProvideAreaMode = FALSE;
							return FALSE;
						}

						m_iBufferAreaStart = m_iBufferFocusPos;
						bOpenArea = TRUE;
					}

					else
					{
						m_bProvideAreaMode = FALSE;
						return FALSE;
					}
				}
			}

			else if (bOpenArea && wpTitleBuffer[0] == L'}')
			{
				m_iBufferAreaEnd == m_iBufferFocusPos - 1;
				m_bProvideAreaMode = FALSE;
				return TRUE;
			}
		}
	}

	m_bProvideAreaMode = FALSE;
	return FALSE;
}

BOOL CAyaINIParserUni::GetValue(WCHAR *szName, WCHAR *szValue, int *ipBufSize)
{

}

BOOL CAyaINIParserUni::GetValue(WCHAR *szName, int *ipValue)
{

}

BOOL CAyaINIParserUni::GetValue(WCHAR *szName, float *fpValue)
{

}

BOOL CAyaINIParserUni::SkipNoneCommand()
{

}

BOOL CAyaINIParserUni::GetNextWord(WCHAR **wppBuffer, int *ipLength)
{
	if (!SkipNoneCommand())		return FALSE;

	WCHAR *wpBuffer = m_wpBuffer + m_iBufferFocusPos;
	*wppBuffer = wpBuffer;

	if (*wpBuffer == L'"')
	{
		if (GetStringWord(wppBuffer, ipLength))	return TRUE;
		else									return FALSE;
	}

	while (1)
	{
		//------------------------------------------------------------------------
		// �ܾ� �����϶� ��ħ
		// ,    : �ĸ�                     "    : ����ǥ
		// 0x20 : �����̽�, ����           0x08 : �齺���̽�
		// 0x09 : ��                       0x0a : ���� �ǵ�, �ٹٲ�
		// 0x0d : ĳ���� ����
		//------------------------------------------------------------------------
		if (*wpBuffer == L',' || *wpBuffer == L'"' || *wpBuffer == 0x20 ||
			*wpBuffer == 0x08 || *wpBuffer == 0x09 || *wpBuffer == 0x0a ||
			*wpBuffer == 0x0d || *wpBuffer == 0)
			break;

		//------------------------------------------------------------------------
		// �ܾ� ���� ���
		//------------------------------------------------------------------------
		m_iBufferFocusPos++;
		(*wpBuffer)++;
		(*ipLength)++;

		//------------------------------------------------------------------------
		// ������ ��
		//------------------------------------------------------------------------
		if (m_iBufferFocusPos > m_iLoadSize)	break;
	}
	
	return TRUE;
}

BOOL CAyaINIParserUni::GetStringWord(WCHAR **wppBuffer, int *ipLength)
{
	WCHAR *wpBuffer = m_wpBuffer + m_iBufferFocusPos;

	//----------------------------------------------------------------------------
	// ó���� ����ǥ�� �ƴϸ� ���ڿ��� �ƴ�
	//----------------------------------------------------------------------------
	if (*wpBuffer != L'"')			return FALSE;

	m_iBufferFocusPos++;
	wpBuffer++;
	(*wppBuffer)++;

	while (1)
	{
		//----------------------------------------------------------------------------
		// ���� ����ǥ���� �˻�
		//----------------------------------------------------------------------------
		if (*wpBuffer == L'"')
		{
			m_iBufferFocusPos++;
			break;
		}

		m_iBufferFocusPos++;
		wpBuffer++;
		(*ipLength)++;

		//------------------------------------------------------------------------
		// ������ ��
		//------------------------------------------------------------------------
		if (m_iBufferFocusPos > m_iLoadSize)	break;
	}

	return TRUE;
}