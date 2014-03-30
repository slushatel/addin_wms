
#include "stdafx.h"

#ifdef __linux__
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#endif

#include <wchar.h>
#include <string>
#include "AddInNative.h"

#include "tinyxml\tinyxml.h"


//#include "TSCLib.h"

static wchar_t *g_PropNames[] = {
	L"IsTest", L"TestProp1"
};
static wchar_t *g_MethodNames[] = { L"Test", L"GetTestData1" };

static const wchar_t g_kClassNames[] = L"CAddInNative"; //|OtherClass1|OtherClass2";

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);

//---------------------------------------------------------------------------//
long GetClassObject(const wchar_t* wsName, IComponentBase** pInterface)
{
	if (!*pInterface)
	{
		*pInterface = new CAddInNative();
		return (long)*pInterface;
	}
	return 0;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
	if (!*pIntf)
		return -1;

	delete *pIntf;
	*pIntf = 0;
	return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
	static WCHAR_T* names = 0;
	if (!names)
		::convToShortWchar(&names, g_kClassNames);
	return names;
}
//---------------------------------------------------------------------------//
//CAddInNative
CAddInNative::CAddInNative()
{
	m_iMemory = 0;
	m_iConnect = 0;
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative()
{
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{
	m_iConnect = (IAddInDefBase*)pConnection;
	return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetInfo()
{
	return 2000;
}
//---------------------------------------------------------------------------//
void CAddInNative::Done()
{
}
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{
	wchar_t *wsExtension = L"AddInNativeExtension";
	int iActualSize = ::wcslen(wsExtension) + 1;
	WCHAR_T* dest = 0;

	if (m_iMemory)
	{
		if (m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
			::convToShortWchar(wsExtensionName, wsExtension, iActualSize);
		return true;
	}

	return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNProps()
{
	return ePropLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T* wsPropName)
{
	long plPropNum = -1;
	wchar_t* propName = 0;

	::convFromShortWchar(&propName, wsPropName);
	plPropNum = findName(g_PropNames, propName, ePropLast);

	//if (plPropNum == -1)
	//    plPropNum = findName(g_PropNamesRu, propName, ePropLast);

	delete[] propName;

	return plPropNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetPropName(long lPropNum, long lPropAlias)
{
	if (lPropNum >= ePropLast)
		return NULL;

	wchar_t *wsCurrentName = NULL;
	WCHAR_T *wsPropName = NULL;
	int iActualSize = 0;

	switch (lPropAlias)
	{
	case 0: // First language
		wsCurrentName = g_PropNames[lPropNum];
		break;
	default:
		return 0;
	}

	iActualSize = wcslen(wsCurrentName) + 1;

	if (m_iMemory && wsCurrentName)
	{
		if (m_iMemory->AllocMemory((void**)&wsPropName, iActualSize * sizeof(WCHAR_T)))
			::convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
	}

	return wsPropName;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
	switch (lPropNum)
	{
	case ePropIsTest:
		TV_VT(pvarPropVal) = VTYPE_BOOL;
		TV_BOOL(pvarPropVal) = m_boolIsTest;
		break;
	default:
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant* varPropVal)
{
	switch (lPropNum)
	{
	case ePropIsTest:
		if (TV_VT(varPropVal) != VTYPE_BOOL)
			return false;
		m_boolIsTest = TV_BOOL(varPropVal);
		break;
	case ePropTestProp1:
	{
						   int size = 0;
						   char *mbstr = 0;
						   wchar_t* wsTmp = 0;

						   switch (TV_VT(varPropVal))
						   {
						   case VTYPE_PSTR:
							   m_TestProp1 = varPropVal->pstrVal;
							   break;
						   case VTYPE_PWSTR:
							   ::convFromShortWchar(&wsTmp, TV_WSTR(varPropVal));
							   size = wcstombs(0, wsTmp, 0) + 1;
							   mbstr = new char[size];
							   memset(mbstr, 0, size);
							   size = wcstombs(mbstr, wsTmp, getLenShortWcharStr(TV_WSTR(varPropVal)));
							   m_TestProp1 = mbstr;
							   break;
						   default:
							   return false;
						   }
	}
		break;
	default:
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum)
{
	switch (lPropNum)
	{
	case ePropIsTest:
		return true;
		break;
	default:
		return false;
	}

	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropWritable(const long lPropNum)
{
	switch (lPropNum)
	{
	case ePropIsTest:
		return true;
	default:
		return true;
	}

	return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNMethods()
{
	return eMethLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindMethod(const WCHAR_T* wsMethodName)
{
	long plMethodNum = -1;
	wchar_t* name = 0;

	::convFromShortWchar(&name, wsMethodName);

	plMethodNum = findName(g_MethodNames, name, eMethLast);

	return plMethodNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetMethodName(const long lMethodNum, const long lMethodAlias)
{
	if (lMethodNum >= eMethLast)
		return NULL;

	wchar_t *wsCurrentName = NULL;
	WCHAR_T *wsMethodName = NULL;
	int iActualSize = 0;

	switch (lMethodAlias)
	{
	case 0: // First language
		wsCurrentName = g_MethodNames[lMethodNum];
		break;
	default:
		return 0;
	}

	iActualSize = wcslen(wsCurrentName) + 1;

	if (m_iMemory && wsCurrentName)
	{
		if (m_iMemory->AllocMemory((void**)&wsMethodName, iActualSize * sizeof(WCHAR_T)))
			::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
	}

	return wsMethodName;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum)
{
	switch (lMethodNum)
	{
	case eMethTest:
		return 0;
	case eMethGetTestData1:
		return 0;
	default:
		return 0;
	}

	return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
	tVariant *pvarParamDefValue)
{
	TV_VT(pvarParamDefValue) = VTYPE_EMPTY;

	switch (lMethodNum)

	{
	case eMethTest:
	case eMethGetTestData1:
		// There are no parameter values by default 
		break;
	default:
		return false;
	}

	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum)
{
	switch (lMethodNum)
	{
	case eMethTest:
		return false;
	case eMethGetTestData1:
		return true;
	default:
		return false;
	}

	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum,
	tVariant* paParams, const long lSizeArray)
{
	switch (lMethodNum)
	{
	case eMethTest:
		m_boolIsTest = !m_boolIsTest;
		break;
	default:
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsFunc(const long lMethodNum,
	tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
	switch (lMethodNum)
	{
	case eMethTest:
		m_boolIsTest = !m_boolIsTest;
		break;
	case eMethGetTestData1:
	{
							  std::string filedata = 
								  R"foo(<?xml version='1.0'>
<table>
<row>
<col>1</col><col>2</col>
</row> 
<row>
<col>11</col><col>12</col>
</row> 
<row>
<col>21</col><col>22</col>
</row> 
								  </table>)foo";
							  TiXmlDocument doc;
							  const char* p = doc.Parse(filedata.c_str(), 0, TIXML_ENCODING_UTF8);

							  std::string rr="";

							  TiXmlElement* table = doc.FirstChildElement("table");
							  TiXmlNode* row = 0;
							  while (row = table->IterateChildren(row)){
								  TiXmlNode* col = 0;
								  while (col = row->IterateChildren(col)){
									  rr += (std::string) (col->FirstChild()->Value());
								  };
							  };

							  std::wstring rr1;
							  rr1.assign(rr.begin(), rr.end());

											  // Finally do something useful.
											  if (!p)
											  {
												  //return 0;
											  }
											  if (!*p)
											  {
												  //return p;
											  }
											  wstring_to_p((std::wstring)rr1, pvarRetValue);
											  break;
										  }
	default:
		return false;
									  }

									  return true;
								  }
								  //---------------------------------------------------------------------------//
								  void CAddInNative::SetLocale(const WCHAR_T* loc)
								  {
#ifndef __linux__
									  _wsetlocale(LC_ALL, loc);
#else
									  int size = 0;
									  char *mbstr = 0;
									  wchar_t *tmpLoc = 0;
									  convFromShortWchar(&tmpLoc, loc);
									  size = wcstombs(0, tmpLoc, 0)+1;
									  mbstr = new char[size];

									  if (!mbstr)
									  {
										  delete[] tmpLoc;
										  return;
									  }

									  memset(mbstr, 0, size);
									  size = wcstombs(mbstr, tmpLoc, wcslen(tmpLoc));
									  setlocale(LC_ALL, mbstr);
									  delete[] tmpLoc;
									  delete[] mbstr;
#endif
								  }
								  //---------------------------------------------------------------------------//
								  bool CAddInNative::setMemManager(void* mem)
								  {
									  m_iMemory = (IMemoryManager*)mem;
									  return m_iMemory != 0;
								  }

								  //---------------------------------------------------------------------------//

								  void CAddInNative::addError(uint32_t wcode, const wchar_t* source,
									  const wchar_t* descriptor, long code)
								  {
									  if (m_iConnect)
									  {
										  WCHAR_T *err = 0;
										  WCHAR_T *descr = 0;

										  ::convToShortWchar(&err, source);
										  ::convToShortWchar(&descr, descriptor);

										  m_iConnect->AddError(wcode, err, descr, code);
										  delete[] err;
										  delete[] descr;
									  }
								  }
								  //---------------------------------------------------------------------------//
								  long CAddInNative::findName(wchar_t* names[], const wchar_t* name,
									  const uint32_t size) const
								  {
									  long ret = -1;
									  for (uint32_t i = 0; i < size; i++)
									  {
										  if (!wcscmp(names[i], name))
										  {
											  ret = i;
											  break;
										  }
									  }
									  return ret;
								  }

								  //---------------------------------------------------------------------------//
								  uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len)
								  {
									  if (!len)
										  len = ::wcslen(Source) + 1;

									  if (!*Dest)
										  *Dest = new WCHAR_T[len];

									  WCHAR_T* tmpShort = *Dest;
									  wchar_t* tmpWChar = (wchar_t*)Source;
									  uint32_t res = 0;

									  ::memset(*Dest, 0, len*sizeof(WCHAR_T));
									  do
									  {
										  *tmpShort++ = (WCHAR_T)*tmpWChar++;
										  ++res;
									  } while (len-- && *tmpWChar);

									  return res;
								  }
								  //---------------------------------------------------------------------------//
								  uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
								  {
									  if (!len)
										  len = getLenShortWcharStr(Source) + 1;

									  if (!*Dest)
										  *Dest = new wchar_t[len];

									  wchar_t* tmpWChar = *Dest;
									  WCHAR_T* tmpShort = (WCHAR_T*)Source;
									  uint32_t res = 0;

									  ::memset(*Dest, 0, len*sizeof(wchar_t));
									  do
									  {
										  *tmpWChar++ = (wchar_t)*tmpShort++;
										  ++res;
									  } while (len-- && *tmpShort);

									  return res;
								  }
								  //---------------------------------------------------------------------------//
								  uint32_t getLenShortWcharStr(const WCHAR_T* Source)
								  {
									  uint32_t res = 0;
									  WCHAR_T *tmpShort = (WCHAR_T*)Source;

									  while (*tmpShort++)
										  ++res;

									  return res;
								  }
								  //---------------------------------------------------------------------------//

								  bool CAddInNative::wstring_to_p(std::wstring str, tVariant* val) {
									  char* t1;
									  TV_VT(val) = VTYPE_PWSTR;
									  m_iMemory->AllocMemory((void**)&t1, (str.length() + 1) * sizeof(WCHAR_T));
									  memcpy(t1, str.c_str(), (str.length() + 1) * sizeof(WCHAR_T));
									  val->pstrVal = t1;
									  val->strLen = str.length();
									  return true;
								  }