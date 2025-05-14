#pragma once
class CAppConfigDb
{
public:
	CAppConfigDb();

	CString m_host;
	unsigned short m_port;
	CString m_name;
	CString m_username;
	CString m_password;
};

class CAppConfigPrize
{
public:
	CAppConfigPrize();

	CString m_username;
	int m_world;
	int m_charguid;
	int m_item;
	int m_count;
};
class CAppConfig
{
public:
	CAppConfig();
	void loadFromFile(LPCTSTR filePath);
	void saveToFile(LPCTSTR filePath);

	CAppConfigDb m_dbConfig;
	CAppConfigPrize m_prizeConfig;
};

