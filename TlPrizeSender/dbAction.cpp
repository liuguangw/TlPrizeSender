#include "pch.h"
#include "dbAction.h"
#include <stdexcept>
#include <mysqld_error.h>
#pragma comment(lib, "libmariadb.lib")

void processSendPrize(const CAppConfigDb& dbConfig, const CString& username, int world, int charguid, int item, int count) {
	MYSQL* conn = nullptr;        // MYSQL ���Ӿ��

	// 1. ��ʼ�� MYSQL ����
	conn = mysql_init(nullptr);
	if (conn == nullptr) {
		throw std::runtime_error("mysql_init() failed. Not enough memory?");
	}
	//����sslУ��
	my_bool verify = 0;
	if (mysql_optionsv(conn, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, (void*)&verify)) {
		fprintf(stderr, "mysql_optionsv(MYSQL_OPT_SSL_VERIFY_SERVER_CERT) failed: %s\n", mysql_error(conn));
		mysql_close(conn);
		// ������
	}
	// 2. ���ӵ����ݿ�
	//���һ������ client_flag ͨ����Ϊ 0�����߸�����Ҫ�����ض���־

	CStringA host(dbConfig.m_host);
	CStringA user(dbConfig.m_username);
	CStringA password(dbConfig.m_password);
	CStringA db_name(dbConfig.m_name);

	if (mysql_real_connect(conn, host, user, password, db_name, dbConfig.m_port, nullptr, 0) == nullptr) {
		std::string errMsg = "mysql_real_connect() failed: ";
		errMsg += mysql_error(conn);
		mysql_close(conn); // ���� MYSQL ����
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

	// 1. ��ʼ��Ԥ���������
	stmt = mysql_stmt_init(conn);
	if (!stmt) {
		std::string err_msg = "Failed to initialize MySQL statement: ";
		err_msg += mysql_error(conn);
		mysql_close(conn);
		throw std::runtime_error(err_msg);
	}

	// 2. ׼��SQL���
	// ʹ�� '?' ��Ϊ����ռλ��
	const std::string query_str = "SELECT 1 FROM account WHERE name = ? LIMIT 1";
	if (mysql_stmt_prepare(stmt, query_str.c_str(), (unsigned long)query_str.length())) {
		std::string err_msg = "Failed to prepare MySQL statement: ";
		err_msg += mysql_stmt_error(stmt); // ����������Ϣ����������
		mysql_stmt_close(stmt);            // �ر������
		mysql_close(conn);                 // �ر�����
		throw std::runtime_error(err_msg);
	}

	// 3. �󶨲���
	MYSQL_BIND param_bind[1];
	memset(param_bind, 0, sizeof(param_bind)); // ���뽫 MYSQL_BIND �ṹ����

	param_bind[0].buffer_type = MYSQL_TYPE_STRING; // ��������Ϊ�ַ���
	param_bind[0].buffer = (char*)account;         // ָ��ʵ�ʲ������� (�˻���)
	param_bind[0].buffer_length = (unsigned long)strlen(account); // �������ݵĳ���
	// param_bind[0].is_null = 0; (��ͨ�� memset ���㣬��ʾ��NULL)
	// param_bind[0].length = nullptr; (���ڹ̶����ȵ������ַ�����ͨ������Ҫ����)

	if (mysql_stmt_bind_param(stmt, param_bind)) {
		std::string err_msg = "Failed to bind parameters for MySQL statement: ";
		err_msg += mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		mysql_close(conn);
		throw std::runtime_error(err_msg);
	}

	// 4. ִ��Ԥ�������
	if (mysql_stmt_execute(stmt)) {
		std::string err_msg = "Failed to execute MySQL statement: ";
		err_msg += mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		mysql_close(conn);
		throw std::runtime_error(err_msg);
	}

	// 5. ��������洢�ڿͻ��ˣ��Ա��ȡ����
	// ���� SELECT ��ѯ�������Ҫ֪����������������У����Ǳ���ġ�
	if (mysql_stmt_store_result(stmt)) {
		std::string err_msg = "Failed to store MySQL statement result: ";
		err_msg += mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		mysql_close(conn);
		throw std::runtime_error(err_msg);
	}

	// 6. ��ȡ��ѯ���������
	my_ulonglong num_rows = mysql_stmt_num_rows(stmt);

	// 7. �ͷ��� mysql_stmt_store_result ����Ľ�����ڴ�
	// �ڵ��� mysql_stmt_store_result ���ɹ����Լ��ڹر����֮ǰ��Ӧ���ô˺�����
	mysql_stmt_free_result(stmt);


	// 8. �ر�Ԥ���������
	// ��Ҳ���ͷ��������ص�������Դ��
	// ���ۺ����˻��Ƿ���ڣ��������Ӧ���رա�
	if (mysql_stmt_close(stmt)) {
		// ����ر���䱾��ʧ�ܣ�����һ���µĴ���
		std::string err_msg = "Failed to close MySQL statement: ";
		// ��ʱ����������Ѵ��ڲ�һ��״̬�������������stmt��conn��
		err_msg += mysql_stmt_error(stmt); // ���Դ�stmt��ȡ����
		if (strlen(mysql_stmt_error(stmt)) == 0) { // ���stmt����Ϊ�գ�����conn����
			err_msg += mysql_error(conn);
		}
		mysql_close(conn); // ��Ȼ��Ҫ�ر�����
		throw std::runtime_error(err_msg); // �׳��ر����ʧ�ܵ��쳣
	}
	stmt = nullptr; // ���������ѹر�

	// 9. ���������ж��˻��Ƿ����
	if (num_rows == 0) {
		// �˻�������
		std::string err_msg = "��Ϸ�˺� '";
		err_msg += account;
		err_msg += "' ������";
		mysql_close(conn); // �ر����ݿ�����
		throw std::runtime_error(err_msg);
	}

	// ��� num_rows > 0 (�ڴ˳�������ζ�� num_rows == 1),
	// ���ʾ�˻����ڡ������������أ����ݿ����� conn ���ִ�״̬��
}

void checkPrizeTable(MYSQL* conn)
{
	const char* check_table_query = "SELECT EXISTS(SELECT 1 FROM account_prize) AS m";

	if (mysql_real_query(conn, check_table_query, (unsigned long)strlen(check_table_query))) {
		// SELECT ��ѯʧ�ܡ�������Ĵ����롣
		unsigned int error_number = mysql_errno(conn);

		if (error_number == ER_NO_SUCH_TABLE) {
			// �������� ER_NO_SUCH_TABLE����ʾ 'account_prize' �����ڡ���������
			const char* create_table_sql =
				"CREATE TABLE account_prize (\n"
				"id bigint(20) NOT NULL AUTO_INCREMENT,\n"
				"account varchar(50) NOT NULL COMMENT '�˺�',\n"
				"world int(11) NOT NULL DEFAULT '0' COMMENT '����ID',\n"
				"charguid int(10) unsigned NOT NULL DEFAULT '0' COMMENT '���GUID',\n"
				"itemid int(10) unsigned NOT NULL DEFAULT '0' COMMENT '��ƷID',\n"
				"itemnum int(11) NOT NULL COMMENT '��Ʒ����',\n"
				"isget smallint(6) NOT NULL COMMENT '�Ƿ���ȡ��',\n"
				"validtime int(11) NOT NULL COMMENT '��Ч�ڣ�ʱ���ʽΪunixʱ��',\n"
				"PRIMARY KEY(id) USING BTREE,\n"
				"UNIQUE KEY id(id) USING BTREE\n"
				")";

			if (mysql_real_query(conn, create_table_sql, (unsigned long)strlen(create_table_sql))) {
				// ִ�� CREATE TABLE ���ʧ�ܡ�
				std::string err_msg = "Failed to create table 'account_prize' (after detecting ER_NO_SUCH_TABLE): ";
				err_msg += mysql_error(conn); // ��ȡ CREATE TABLE �����Ĵ�����Ϣ
				mysql_close(conn);            // �ر�����
				throw std::runtime_error(err_msg); // �׳��쳣
			}
			// �� 'account_prize' �ɹ�������
			// �����������������ӱ��ִ򿪡�
			return;
		}
		else {
			// SELECT ��ѯʧ�ܣ�������ԭ���� ER_NO_SUCH_TABLE��
			// �ⱻ��Ϊһ��δԤ�ϵ��Ĵ���
			char buffer[15] = { 0 };
			_itoa_s(error_number, buffer, 10);
			std::string err_msg = "Error when checking existence of 'account_prize' with SELECT: ";
			err_msg += mysql_error(conn); // ��ȡ SELECT �����Ĵ�����Ϣ
			err_msg += " (Error Code: ";
			err_msg += buffer;
			err_msg += ")"; // ���Ӵ������Ա����
			mysql_close(conn);             // �ر�����
			throw std::runtime_error(err_msg); // �׳��쳣
		}
	}
	// SELECT ��ѯ�ɹ�ִ�У�����ζ�� 'account_prize' ���Ѵ��ڡ�
	// ��ʹ���ǲ�ֱ��ʹ�� SELECT �Ľ����Ҳ���봦�������������״̬��Ϊ������ѯ��׼����
	MYSQL_RES* result = mysql_store_result(conn);
	if (result == nullptr) {
		// ��� mysql_real_query �ɹ����� mysql_store_result �Լ򵥵� SELECT 1 ʧ�ܣ�
		// ��ͨ����ʾ�ͻ��˳������⣨�����ڴ治�㣩��
		std::string err_msg = "Failed to process/store result from 'SELECT 1 FROM account_prize' (table exists): ";
		err_msg += mysql_error(conn);
		mysql_close(conn);
		throw std::runtime_error(err_msg);
	}
	mysql_free_result(result);
}

void insertPrize(MYSQL* conn, const char* account, int world, int charguid, int item, int count)
{
	MYSQL_STMT* stmt = nullptr;   // MYSQL �����

	// 1. ׼�� SQL �������
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

	// 2. �󶨲���
	MYSQL_BIND params[6];
	memset(params, 0, sizeof(params)); // ��ʼ�� MYSQL_BIND �ṹ������

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
	params[paramIndex].length = 0; // ������ֵ���ͣ����ֶ�δʹ��
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

	// ��ȡ��ǰϵͳʱ��
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

	// 3. ִ�����
	if (mysql_stmt_execute(stmt) != 0) {
		std::string errMsg = "mysql_stmt_execute() failed: ";
		errMsg += mysql_stmt_error(stmt);
		// ����Ƿ����ظ������� (ER_DUP_ENTRY, ����� 1062)
		// unsigned int err_no = mysql_stmt_errno(stmt);
		// if (err_no == 1062) { /* ���⴦���ظ��� */ }
		mysql_stmt_close(stmt);
		mysql_close(conn);
		throw std::runtime_error(errMsg);
	}

	// ����ѡ����ȡ��Ӱ�������
	// my_ulonglong affected_rows = mysql_stmt_affected_rows(stmt);
	// if (affected_rows == 1) {
	//     // ����ɹ�
	// }

	// 4. ������Դ
	if (mysql_stmt_close(stmt) != 0) {
		// ��Ȼ�ر������ʧ�ܲ������������Լ�¼��־
		// ͨ����ʱ��Ӧ����֮ǰ�ĳɹ�״̬���׳����쳣����������
		// ������ϸ�Ҫ��Ҳ�����׳��쳣
		// std::string errMsg = "mysql_stmt_close() failed: ";
		// errMsg += mysql_error(conn); // stmt �Ĵ�������� mysql_stmt_error(stmt)
		// mysql_close(conn);
		// throw std::runtime_error(errMsg);
	}
}
