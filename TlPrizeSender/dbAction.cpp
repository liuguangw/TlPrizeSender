#include "pch.h"
#include "dbAction.h"
#include <stdexcept>
#include <mysqld_error.h>
#pragma comment(lib, "libmariadb.lib")

void processSendPrize(const CAppConfigDb& dbConfig, const CString& username, int world, int charguid, int item, int count) {
	MYSQL* conn = nullptr;        // MYSQL 连接句柄

	// 1. 初始化 MYSQL 对象
	conn = mysql_init(nullptr);
	if (conn == nullptr) {
		throw std::runtime_error("mysql_init() failed. Not enough memory?");
	}
	//禁用ssl校验
	my_bool verify = 0;
	if (mysql_optionsv(conn, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, (void*)&verify)) {
		fprintf(stderr, "mysql_optionsv(MYSQL_OPT_SSL_VERIFY_SERVER_CERT) failed: %s\n", mysql_error(conn));
		mysql_close(conn);
		// 错误处理
	}
	// 2. 连接到数据库
	//最后一个参数 client_flag 通常设为 0，或者根据需要设置特定标志

	CStringA host(dbConfig.m_host);
	CStringA user(dbConfig.m_username);
	CStringA password(dbConfig.m_password);
	CStringA db_name(dbConfig.m_name);

	if (mysql_real_connect(conn, host, user, password, db_name, dbConfig.m_port, nullptr, 0) == nullptr) {
		std::string errMsg = "mysql_real_connect() failed: ";
		errMsg += mysql_error(conn);
		mysql_close(conn); // 清理 MYSQL 对象
		throw std::runtime_error(errMsg);
	}
	CStringA account(username);
	const char* accountStr = account.GetString();

	checkAccountExists(conn, accountStr);
	checkPrizeTable(conn);
	insertPrize(conn, accountStr, world, charguid, item, count);

	mysql_close(conn);
}

void checkAccountExists(MYSQL* conn, const char* account)
{
	MYSQL_STMT* stmt = nullptr;

	// 1. 初始化预处理语句句柄
	stmt = mysql_stmt_init(conn);
	if (!stmt) {
		std::string err_msg = "Failed to initialize MySQL statement: ";
		err_msg += mysql_error(conn);
		mysql_close(conn);
		throw std::runtime_error(err_msg);
	}

	// 2. 准备SQL语句
	// 使用 '?' 作为参数占位符
	const std::string query_str = "SELECT 1 FROM account WHERE name = ? LIMIT 1";
	if (mysql_stmt_prepare(stmt, query_str.c_str(), (unsigned long)query_str.length())) {
		std::string err_msg = "Failed to prepare MySQL statement: ";
		err_msg += mysql_stmt_error(stmt); // 后续错误信息来自语句对象
		mysql_stmt_close(stmt);            // 关闭语句句柄
		mysql_close(conn);                 // 关闭连接
		throw std::runtime_error(err_msg);
	}

	// 3. 绑定参数
	MYSQL_BIND param_bind[1];
	memset(param_bind, 0, sizeof(param_bind)); // 必须将 MYSQL_BIND 结构清零

	param_bind[0].buffer_type = MYSQL_TYPE_STRING; // 参数类型为字符串
	param_bind[0].buffer = (char*)account;         // 指向实际参数数据 (账户名)
	param_bind[0].buffer_length = (unsigned long)strlen(account); // 参数数据的长度
	// param_bind[0].is_null = 0; (已通过 memset 清零，表示非NULL)
	// param_bind[0].length = nullptr; (对于固定长度的输入字符串，通常不需要设置)

	if (mysql_stmt_bind_param(stmt, param_bind)) {
		std::string err_msg = "Failed to bind parameters for MySQL statement: ";
		err_msg += mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		mysql_close(conn);
		throw std::runtime_error(err_msg);
	}

	// 4. 执行预处理语句
	if (mysql_stmt_execute(stmt)) {
		std::string err_msg = "Failed to execute MySQL statement: ";
		err_msg += mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		mysql_close(conn);
		throw std::runtime_error(err_msg);
	}

	// 5. 将结果集存储在客户端，以便获取行数
	// 对于 SELECT 查询，如果需要知道行数或随机访问行，这是必需的。
	if (mysql_stmt_store_result(stmt)) {
		std::string err_msg = "Failed to store MySQL statement result: ";
		err_msg += mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		mysql_close(conn);
		throw std::runtime_error(err_msg);
	}

	// 6. 获取查询结果的行数
	my_ulonglong num_rows = mysql_stmt_num_rows(stmt);

	// 7. 释放由 mysql_stmt_store_result 分配的结果集内存
	// 在调用 mysql_stmt_store_result 并成功后，以及在关闭语句之前，应调用此函数。
	mysql_stmt_free_result(stmt);


	// 8. 关闭预处理语句句柄
	// 这也会释放与语句相关的其他资源。
	// 无论后续账户是否存在，语句句柄都应被关闭。
	if (mysql_stmt_close(stmt)) {
		// 如果关闭语句本身失败，这是一个新的错误。
		std::string err_msg = "Failed to close MySQL statement: ";
		// 此时语句句柄可能已处于不一致状态，错误可能仍在stmt或conn上
		err_msg += mysql_stmt_error(stmt); // 尝试从stmt获取错误
		if (strlen(mysql_stmt_error(stmt)) == 0) { // 如果stmt错误为空，尝试conn错误
			err_msg += mysql_error(conn);
		}
		mysql_close(conn); // 仍然需要关闭连接
		throw std::runtime_error(err_msg); // 抛出关闭语句失败的异常
	}
	stmt = nullptr; // 标记语句句柄已关闭

	// 9. 根据行数判断账户是否存在
	if (num_rows == 0) {
		// 账户不存在
		std::string err_msg = "游戏账号 '";
		err_msg += account;
		err_msg += "' 不存在";
		mysql_close(conn); // 关闭数据库连接
		throw std::runtime_error(err_msg);
	}

	// 如果 num_rows > 0 (在此场景下意味着 num_rows == 1),
	// 则表示账户存在。函数正常返回，数据库连接 conn 保持打开状态。
}

void checkPrizeTable(MYSQL* conn)
{
	const char* check_table_query = "SELECT EXISTS(SELECT 1 FROM account_prize) AS m";

	if (mysql_real_query(conn, check_table_query, (unsigned long)strlen(check_table_query))) {
		// SELECT 查询失败。检查具体的错误码。
		unsigned int error_number = mysql_errno(conn);

		if (error_number == ER_NO_SUCH_TABLE) {
			// 错误码是 ER_NO_SUCH_TABLE，表示 'account_prize' 表不存在。创建它。
			const char* create_table_sql =
				"CREATE TABLE account_prize (\n"
				"id bigint(20) NOT NULL AUTO_INCREMENT,\n"
				"account varchar(50) NOT NULL COMMENT '账号',\n"
				"world int(11) NOT NULL DEFAULT '0' COMMENT '世界ID',\n"
				"charguid int(10) unsigned NOT NULL DEFAULT '0' COMMENT '玩家GUID',\n"
				"itemid int(10) unsigned NOT NULL DEFAULT '0' COMMENT '物品ID',\n"
				"itemnum int(11) NOT NULL COMMENT '物品数量',\n"
				"isget smallint(6) NOT NULL COMMENT '是否领取了',\n"
				"validtime int(11) NOT NULL COMMENT '有效期，时间格式为unix时间',\n"
				"PRIMARY KEY(id) USING BTREE,\n"
				"UNIQUE KEY id(id) USING BTREE\n"
				")";

			if (mysql_real_query(conn, create_table_sql, (unsigned long)strlen(create_table_sql))) {
				// 执行 CREATE TABLE 语句失败。
				std::string err_msg = "Failed to create table 'account_prize' (after detecting ER_NO_SUCH_TABLE): ";
				err_msg += mysql_error(conn); // 获取 CREATE TABLE 操作的错误信息
				mysql_close(conn);            // 关闭连接
				throw std::runtime_error(err_msg); // 抛出异常
			}
			// 表 'account_prize' 成功创建。
			// 函数正常结束，连接保持打开。
			return;
		}
		else {
			// SELECT 查询失败，但错误原因不是 ER_NO_SUCH_TABLE。
			// 这被视为一个未预料到的错误。
			char buffer[15] = { 0 };
			_itoa_s(error_number, buffer, 10);
			std::string err_msg = "Error when checking existence of 'account_prize' with SELECT: ";
			err_msg += mysql_error(conn); // 获取 SELECT 操作的错误信息
			err_msg += " (Error Code: ";
			err_msg += buffer;
			err_msg += ")"; // 附加错误码以便调试
			mysql_close(conn);             // 关闭连接
			throw std::runtime_error(err_msg); // 抛出异常
		}
	}
	// SELECT 查询成功执行，这意味着 'account_prize' 表已存在。
	// 即使我们不直接使用 SELECT 的结果，也必须处理它以清空连接状态，为后续查询做准备。
	MYSQL_RES* result = mysql_store_result(conn);
	if (result == nullptr) {
		// 如果 mysql_real_query 成功，但 mysql_store_result 对简单的 SELECT 1 失败，
		// 这通常表示客户端出现问题（例如内存不足）。
		std::string err_msg = "Failed to process/store result from 'SELECT 1 FROM account_prize' (table exists): ";
		err_msg += mysql_error(conn);
		mysql_close(conn);
		throw std::runtime_error(err_msg);
	}
	mysql_free_result(result);
}

void insertPrize(MYSQL* conn, const char* account, int world, int charguid, int item, int count)
{
	MYSQL_STMT* stmt = nullptr;   // MYSQL 语句句柄

	// 1. 准备 SQL 插入语句
	const char* sql_query = "INSERT INTO account_prize (account, world, charguid, itemid, itemnum, isget, validtime) VALUES (?, ?, ?, ?, ?, 0, ?)";

	stmt = mysql_stmt_init(conn);
	if (stmt == nullptr) {
		std::string errMsg = "mysql_stmt_init() failed: ";
		errMsg += mysql_error(conn);
		mysql_close(conn);
		throw std::runtime_error(errMsg);
	}

	if (mysql_stmt_prepare(stmt, sql_query, (unsigned long)strlen(sql_query)) != 0) {
		std::string errMsg = "mysql_stmt_prepare() failed: ";
		errMsg += mysql_stmt_error(stmt);
		//1146 ER_NO_SUCH_TABLE
		//unsigned int err_no = mysql_stmt_errno(stmt);
		//TRACE("err=%d\n", err_no);
		mysql_stmt_close(stmt);
		mysql_close(conn);
		throw std::runtime_error(errMsg);
	}

	// 2. 绑定参数
	MYSQL_BIND params[6];
	memset(params, 0, sizeof(params)); // 初始化 MYSQL_BIND 结构体数组

	int paramIndex = 0;

	params[paramIndex].buffer_type = FIELD_TYPE_STRING;
	params[paramIndex].buffer = (char*)account;
	params[paramIndex].buffer_length = (unsigned long)strlen(account);
	params[paramIndex].is_null = 0;
	params[paramIndex].length = (unsigned long*)&params[paramIndex].buffer_length;
	paramIndex++;

	//world
	params[paramIndex].buffer_type = FIELD_TYPE_LONG;
	params[paramIndex].buffer = (char*)&world;
	params[paramIndex].is_null = 0;
	params[paramIndex].length = 0; // 对于数值类型，此字段未使用
	paramIndex++;

	//charguid
	params[paramIndex].buffer_type = FIELD_TYPE_LONG;
	params[paramIndex].buffer = (char*)&charguid;
	params[paramIndex].is_null = 0;
	params[paramIndex].length = 0;
	paramIndex++;

	//item
	params[paramIndex].buffer_type = FIELD_TYPE_LONG;
	params[paramIndex].buffer = (char*)&item;
	params[paramIndex].is_null = 0;
	params[paramIndex].length = 0;
	paramIndex++;

	//count
	params[paramIndex].buffer_type = FIELD_TYPE_LONG;
	params[paramIndex].buffer = (char*)&count;
	params[paramIndex].is_null = 0;
	params[paramIndex].length = 0;
	paramIndex++;

	// 获取当前系统时间
	CTime currentTime = CTime::GetCurrentTime();
	time_t timestamp = currentTime.GetTime();
	int timestampU4 = (int)(timestamp & 0xFFFFFFFF);
	params[paramIndex].buffer_type = FIELD_TYPE_LONG;
	params[paramIndex].buffer = (char*)&timestampU4;
	params[paramIndex].is_null = 0;
	params[paramIndex].length = 0;
	paramIndex++;

	if (mysql_stmt_bind_param(stmt, params) != 0) {
		std::string errMsg = "mysql_stmt_bind_param() failed: ";
		errMsg += mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		mysql_close(conn);
		throw std::runtime_error(errMsg);
	}

	// 3. 执行语句
	if (mysql_stmt_execute(stmt) != 0) {
		std::string errMsg = "mysql_stmt_execute() failed: ";
		errMsg += mysql_stmt_error(stmt);
		// 检查是否是重复键错误 (ER_DUP_ENTRY, 错误号 1062)
		// unsigned int err_no = mysql_stmt_errno(stmt);
		// if (err_no == 1062) { /* 特殊处理重复键 */ }
		mysql_stmt_close(stmt);
		mysql_close(conn);
		throw std::runtime_error(errMsg);
	}

	// （可选）获取受影响的行数
	// my_ulonglong affected_rows = mysql_stmt_affected_rows(stmt);
	// if (affected_rows == 1) {
	//     // 插入成功
	// }

	// 4. 清理资源
	if (mysql_stmt_close(stmt) != 0) {
		// 虽然关闭语句句柄失败不常见，但可以记录日志
		// 通常此时不应覆盖之前的成功状态或抛出新异常干扰主流程
		// 但如果严格要求，也可以抛出异常
		// std::string errMsg = "mysql_stmt_close() failed: ";
		// errMsg += mysql_error(conn); // stmt 的错误可以用 mysql_stmt_error(stmt)
		// mysql_close(conn);
		// throw std::runtime_error(errMsg);
	}
}
