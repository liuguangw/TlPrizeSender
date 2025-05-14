#pragma once
#include "CAppConfig.h"
#include <mysql.h>

// 发放活动奖励
void processSendPrize(const CAppConfigDb& dbConfig, const CString& username, int world, int charguid, int item, int count);
// 检查账号是否存在
void checkAccountExists(MYSQL* conn, const char* account);
// 检查数据表是否存在,如果不存在则自动创建表
void checkPrizeTable(MYSQL* conn);
// insert
void insertPrize(MYSQL* conn, const char* account, int world, int charguid, int item, int count);