#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.h"

using namespace std;

connection_pool::connection_pool() {
    m_CurConn = 0;
    m_FreeConn = 0;
}

connection_pool *connection_pool::GetInstance() {
    static connection_pool connPool;
    return &connPool;
}

//构造初始化
void
connection_pool::init(string url, string User, string PassWord, string DBName,
                      int Port, int MaxConn, int close_log) {
    // 初始化数据库信息
    m_url = url;
    m_Port = Port;
    m_User = User;
    m_PassWord = PassWord;
    m_DatabaseName = DBName;
    m_close_log = close_log;

    //创建MaxConn条数据库连接
    for (int i = 0; i < MaxConn; i++) {
        MYSQL *con = NULL;
        con = mysql_init(con); //分配或初始化与mysql_real_connect()相适应的MYSQL对象

        if (con == NULL) {
            LOG_ERROR("MySQL Error");
            exit(1);
        }
        // 尝试与运行在主机上的MySQL数据库引擎建立连接
        // 如果连接成功，返回MYSQL*连接句柄。 如果失败，返回NULL
        con = mysql_real_connect(con, url.c_str(), User.c_str(),
                                 PassWord.c_str(), DBName.c_str(), Port,
                                 NULL, 0);
        if (con == NULL) {
            LOG_ERROR("MySQL Error");
            exit(1);
        }
        connList.push_back(con);
        ++m_FreeConn;
    }
    // 将信号量初始化为最大连接次数
    // 使用信号量实现多线程争夺连接的同步机制
    reserve = sem(m_FreeConn);

    m_MaxConn = m_FreeConn;
}


//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL *connection_pool::GetConnection() {
    MYSQL *con = NULL;

    if (0 == connList.size())
        return NULL;

    // 取出连接，信号量原子减1，为0则等待
    // If the semaphore currently has the value zero, then the call blocks
    reserve.wait();

    lock.lock();

    con = connList.front();
    connList.pop_front();

    --m_FreeConn;
    ++m_CurConn;

    lock.unlock();
    return con;
}

//释放当前使用的连接
bool connection_pool::ReleaseConnection(MYSQL *con) {
    if (NULL == con)
        return false;

    lock.lock();

    connList.push_back(con);
    ++m_FreeConn;
    --m_CurConn;

    lock.unlock();

    reserve.post(); // increments (unlocks) the semaphore pointed to by sem.
    return true;
}

// 销毁数据库连接池
// 通过迭代器遍历连接池链表，关闭对应数据库连接，清空链表并重置空闲连接和现有连接数量。
void connection_pool::DestroyPool() {

    lock.lock();
    if (!connList.empty()) {
        list<MYSQL *>::iterator it;
        for (it = connList.begin(); it != connList.end(); ++it) {
            MYSQL *con = *it;
            mysql_close(con);
        }
        m_CurConn = 0;
        m_FreeConn = 0;
        connList.clear();
    }

    lock.unlock();
}

//当前空闲的连接数
int connection_pool::GetFreeConn() {
    return this->m_FreeConn;
}

// RAII机制销毁连接池（不是由外部调用，而是自动销毁的）
connection_pool::~connection_pool() {
    DestroyPool();
}

// 数据库连接本身是指针类型，所以参数需要通过双指针才能对其进行修改
connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *connPool) {
    *SQL = connPool->GetConnection();

    conRAII = *SQL;
    poolRAII = connPool;
}

connectionRAII::~connectionRAII() {
    poolRAII->ReleaseConnection(conRAII);
}