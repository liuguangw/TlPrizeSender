#include "pch.h"
#include "CAppConfig.h"

CAppConfigDb::CAppConfigDb() :m_host(_T("")), m_port(0), m_name(_T(""))
, m_username(_T("")), m_password(_T(""))
{
}

CAppConfigPrize::CAppConfigPrize() :m_username(_T("")), m_world(0)
, m_charguid(0), m_item(0), m_count(0)
{
}

CAppConfig::CAppConfig()
{
}

void CAppConfig::loadFromFile(LPCTSTR filePath)
{
	//[db]
	LPCTSTR lpAppName = _T("db");
	TCHAR strBuff[150];
	DWORD nSize = 150;

	GetPrivateProfileString(lpAppName, _T("host"), _T(""), strBuff, nSize, filePath);
	m_dbConfig.m_host = strBuff;
	m_dbConfig.m_port = (unsigned short)GetPrivateProfileInt(lpAppName, _T("port"), 3306, filePath);
	GetPrivateProfileString(lpAppName, _T("name"), _T(""), strBuff, nSize, filePath);
	m_dbConfig.m_name = strBuff;
	GetPrivateProfileString(lpAppName, _T("username"), _T(""), strBuff, nSize, filePath);
	m_dbConfig.m_username = strBuff;
	GetPrivateProfileString(lpAppName, _T("password"), _T(""), strBuff, nSize, filePath);
	m_dbConfig.m_password = strBuff;

	//[prize]
	lpAppName = _T("prize");
	GetPrivateProfileString(lpAppName, _T("username"), _T(""), strBuff, nSize, filePath);
	m_prizeConfig.m_username = strBuff;
	m_prizeConfig.m_world = GetPrivateProfileInt(lpAppName, _T("world"), 0, filePath);
	m_prizeConfig.m_charguid = GetPrivateProfileInt(lpAppName, _T("charguid"), 0, filePath);
	m_prizeConfig.m_item = GetPrivateProfileInt(lpAppName, _T("item"), 0, filePath);
	m_prizeConfig.m_count = GetPrivateProfileInt(lpAppName, _T("count"), 0, filePath);
}

void CAppConfig::saveToFile(LPCTSTR filePath)
{
	//[db]
	LPCTSTR lpAppName = _T("db");
	WritePrivateProfileString(lpAppName, _T("host"), m_dbConfig.m_host, filePath);
	CString strValue;
	strValue.Format(_T("%d"), m_dbConfig.m_port);
	WritePrivateProfileString(lpAppName, _T("port"), strValue, filePath);
	WritePrivateProfileString(lpAppName, _T("name"), m_dbConfig.m_name, filePath);
	WritePrivateProfileString(lpAppName, _T("username"), m_dbConfig.m_username, filePath);
	WritePrivateProfileString(lpAppName, _T("password"), m_dbConfig.m_password, filePath);

	//[prize]
	lpAppName = _T("prize");
	WritePrivateProfileString(lpAppName, _T("username"), m_prizeConfig.m_username, filePath);
	strValue.Format(_T("%d"), m_prizeConfig.m_world);
	WritePrivateProfileString(lpAppName, _T("world"), strValue, filePath);
	strValue.Format(_T("%d"), m_prizeConfig.m_charguid);
	WritePrivateProfileString(lpAppName, _T("charguid"), strValue, filePath);
	strValue.Format(_T("%d"), m_prizeConfig.m_item);
	WritePrivateProfileString(lpAppName, _T("item"), strValue, filePath);
	strValue.Format(_T("%d"), m_prizeConfig.m_count);
	WritePrivateProfileString(lpAppName, _T("count"), strValue, filePath);
}
