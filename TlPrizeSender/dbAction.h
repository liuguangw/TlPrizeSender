#pragma once
#include "CAppConfig.h"
#include <mysql.h>

// ���Ż����
void processSendPrize(const CAppConfigDb& dbConfig, const CString& username, int world, int charguid, int item, int count);
// ����˺��Ƿ����
void checkAccountExists(MYSQL* conn, const char* account);
// ������ݱ��Ƿ����,������������Զ�������
void checkPrizeTable(MYSQL* conn);
// insert
void insertPrize(MYSQL* conn, const char* account, int world, int charguid, int item, int count);