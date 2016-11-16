#include <stdio.h>
#include <Windows.h>
#include "INIParserForUnicode.h"

/*------------------------------------------------------------------------------*/
//생성자
/*------------------------------------------------------------------------------*/
CAyaINIParserUni::CAyaINIParserUni()
{
	m_wpBuffer = new WCHAR[eBUFFER_SIZE];
	Initial();
}

/*------------------------------------------------------------------------------*/
// 파괴자
/*------------------------------------------------------------------------------*/
CAyaINIParserUni::~CAyaINIParserUni()
{
	delete[] m_wpBuffer;
}

/*------------------------------------------------------------------------------*/
// 초기화
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
// 파일 로드
/*------------------------------------------------------------------------------*/
BOOL CAyaINIParserUni::LoadFile(WCHAR *szFileName)
{
	FILE *fp;
	DWORD dReadSize;

	//----------------------------------------------------------------------------
	// 파일 오픈 및 여부 검사
	//----------------------------------------------------------------------------
	_wfopen_s(&fp, szFileName, L"r,ccs=UTF-8");
	if (fp == NULL)
	{
		wprintf(L"Don't exist file...\n");
		return FALSE;
	}

	//----------------------------------------------------------------------------
	// BOM 코드 제거
	//----------------------------------------------------------------------------
	rewind(fp); 

	//----------------------------------------------------------------------------
	// 파일 사이즈 측정
	//----------------------------------------------------------------------------
	fseek(fp, 0, SEEK_END);
	m_iLoadSize = ftell(fp);
	rewind(fp);

	//----------------------------------------------------------------------------
	// 파일 사이즈가 버퍼보다 클 시
	//----------------------------------------------------------------------------
	if (m_iLoadSize > eBUFFER_SIZE)
	{
		wprintf(L"File is too large...\n");
		fclose(fp);
		Initial();
		return FALSE;
	}

	//----------------------------------------------------------------------------
	// 파일 내용 전부 읽기
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
// 구역 찾기
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
		// 구역 시그널 찾으면
		//------------------------------------------------------------------------
		if (wpTitleBuffer[0] == L':')
		{ 
			//--------------------------------------------------------------------
			// 구역 이름 찾기
			//--------------------------------------------------------------------
			if (0 == wcscmp(++wpTitleBuffer, szAreaName))
			{
				if (GetNextWord(&wpSpaceBuffer, &iLength))
				{
					//------------------------------------------------------------
					// '{' 인지 검사
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
		// 단어 기준일때 마침
		// ,    : 컴마                     "    : 따옴표
		// 0x20 : 스페이스, 공백           0x08 : 백스페이스
		// 0x09 : 탭                       0x0a : 라인 피드, 줄바꿈
		// 0x0d : 캐리지 리턴
		//------------------------------------------------------------------------
		if (*wpBuffer == L',' || *wpBuffer == L'"' || *wpBuffer == 0x20 ||
			*wpBuffer == 0x08 || *wpBuffer == 0x09 || *wpBuffer == 0x0a ||
			*wpBuffer == 0x0d || *wpBuffer == 0)
			break;

		//------------------------------------------------------------------------
		// 단어 길이 계산
		//------------------------------------------------------------------------
		m_iBufferFocusPos++;
		(*wpBuffer)++;
		(*ipLength)++;

		//------------------------------------------------------------------------
		// 파일의 끝
		//------------------------------------------------------------------------
		if (m_iBufferFocusPos > m_iLoadSize)	break;
	}
	
	return TRUE;
}

BOOL CAyaINIParserUni::GetStringWord(WCHAR **wppBuffer, int *ipLength)
{
	WCHAR *wpBuffer = m_wpBuffer + m_iBufferFocusPos;

	//----------------------------------------------------------------------------
	// 처음이 따옴표가 아니면 문자열이 아님
	//----------------------------------------------------------------------------
	if (*wpBuffer != L'"')			return FALSE;

	m_iBufferFocusPos++;
	wpBuffer++;
	(*wppBuffer)++;

	while (1)
	{
		//----------------------------------------------------------------------------
		// 다음 따옴표까지 검사
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
		// 파일의 끝
		//------------------------------------------------------------------------
		if (m_iBufferFocusPos > m_iLoadSize)	break;
	}

	return TRUE;
}