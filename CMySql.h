#pragma once
#include<Windows.h>
#include"C:\\Program Files\\MySQL\\MySQL Server 5.7\\include\\mysql.h"
#pragma comment(lib,".\\libmysql.lib")



#include <list>
#include <string>
using namespace std;

class CMySql
{
public:
    CMySql(void);
    ~CMySql(void);
public:
    bool  ConnectMySql(const char *host,const char *user,const char *pass,const char *db);
    void  DisConnect();
    bool  SelectMySql(const char* szSql,int nColumn,list<string>& lstStr);
   
    //更新：删除、插入、修改
    bool  UpdateMySql(const char* szSql);
 
private:
    MYSQL *sock;   
	MYSQL_RES *results;   
	MYSQL_ROW record; 
   
};

