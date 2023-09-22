#pragma once
#ifndef PACKDEF_H
#define PACKDEF_H

#define _DEF_PROTOCOL_BASE   10
//注册
#define _DEF_PROTOCOL_REGISTER_RQ   _DEF_PROTOCOL_BASE +1
#define _DEF_PROTOCOL_REGISTER_RS   _DEF_PROTOCOL_BASE +2
//登录
#define _DEF_PROTOCOL_LOGIN_RQ   _DEF_PROTOCOL_BASE +3
#define _DEF_PROTOCOL_LOGIN_RS   _DEF_PROTOCOL_BASE +4
//获取文件列表
#define _DEF_PROTOCOL_GETFILELIST_RQ   _DEF_PROTOCOL_BASE +5
#define _DEF_PROTOCOL_GETFILELIST_RS   _DEF_PROTOCOL_BASE +6
//上传文件信息
#define _DEF_PROTOCOL_UPLOAD_FILEINFO_RQ   _DEF_PROTOCOL_BASE +7
#define _DEF_PROTOCOL_UPLOAD_FILEINFO_RS   _DEF_PROTOCOL_BASE +8
//上传文件内容
#define _DEF_PROTOCOL_UPLOAD_FILECONTENT_RQ   _DEF_PROTOCOL_BASE +9
#define _DEF_PROTOCOL_UPLOAD_FILECONTENT_RS   _DEF_PROTOCOL_BASE +10



//下载文件请求
#define _DEF_PROTOCOL_DOWNLOAD_FILEINFO_RQ   _DEF_PROTOCOL_BASE +11
#define _DEF_PROTOCOL_DOWNLOAD_FILEINFO_RS   _DEF_PROTOCOL_BASE +12
//下载文件内容
#define _DEF_PROTOCOL_DOWNLOAD_FILECONTENT_SEND   _DEF_PROTOCOL_BASE +13
#define _DEF_PROTOCOL_DOWNLOAD_FILECONTENT_RELY   _DEF_PROTOCOL_BASE +14

//删除文件
#define _DEF_PROTOCOL_DELETE_FILE_RQ   _DEF_PROTOCOL_BASE +15
#define _DEF_PROTOCOL_DELETE_FILE_RS   _DEF_PROTOCOL_BASE +16






#define _upload_file_exists 0 //文件我自己已经上传过，不需要重复上传
#define _upload_file_continue 1 //曾经传过，没有传完。断点续传
#define _upload_file_success 2 // 秒传
#define _upload_file_normal 3 //正常传



#define MAXSIZE  45
#define MAXFILENUM  500
#define MAXSQLLEN  500
#define MAXFILECONTENT 4096

//协议包
#define _register_failed   0
#define _register_success  1

#define _login_user_noexist  0
#define _login_password_err  1
#define _login_success       2


#define _delete_faild 0
#define _delete_success 1


#define _install_pause 0
#define _install_continue 1

//注册请求包
struct STRU_REGISTER_RQ
{
    char m_nType;
    char m_szName[MAXSIZE];
    char m_szPassword[MAXSIZE];
    long long m_ltel;
};

//注册回复包
struct STRU_REGISTER_RS
{
    char m_nType;
    char m_szResult;
};

//登录请求包
struct STRU_LOGIN_RQ
{
    char m_nType;
    char m_szName[MAXSIZE];
    char m_szPassword[MAXSIZE];
};


//登录回复包
struct STRU_LOGIN_RS
{
    char m_nType;
    char m_szResult;
    int m_luserid;
};


//获取文件列表请求
struct STRU_GETFILELIST_RQ
{
    char m_nType;
    int m_luserid;
};

//文件信息
struct FileInfo
{
    char m_szFileName[MAXSIZE];//文件名
    int m_fileSize;//文件大小
    char m_szDateTime[MAXSIZE];//文件日期
};

//获取文件列表回复
struct STRU_GETFILELIST_RS
{
    char m_nType;
    FileInfo m_aryFile[MAXFILENUM];
    int m_nFileNum;
};


//上传文件信息请求
struct STRU_UPLOAD_FILEINFO_RQ
{
    char m_nType;
    int m_lUserId;
    int m_lFileSize;
    char m_szFileName[MAXSIZE];
    char m_szFileMD5[MAXSIZE];

};



//上传文件信息回复
struct STRU_UPLOAD_FILEINFO_RS
{
    char m_nType;
    int m_lUserId;
    int m_lFileId;
    int m_lPosition;
    char m_szFileMD5[MAXSIZE];
    int bresult;

};


//上传文件包
struct STRU_UPLOAD_FILECONTENT_RQ
{
    char m_nType;
    int m_lUserId;
    int m_lFileId;
    char m_szFileContent[MAXFILECONTENT];
    int m_nNum;//大小
    int m_pos;//文件包开始位置
};


//下载文件请求
struct STRU_DOWNLOAD_FILEINFO_RQ
{
    char m_nType;
    int m_lUserId;
    char m_szFileName[MAXSIZE];

};

//下载文件请求回复
struct STRU_DOWNLOAD_FILEINFO_RS
{
    char m_nType;
    int m_lUserId;
    int m_lFileId;
    int m_lPosition;
    int bresult;
    int m_fisize;
    char m_szFileName[MAXSIZE];
};


//下载文件包
struct STRU_DOWNLOAD_FILECONTENT_RQ
{
    char m_nType;
    int m_lUserId;
    int m_lFileId;
    char m_szFileContent[MAXFILECONTENT];
    int m_nNum;//大小
};

//下载文件内容回复包
struct STRU_DOWNLOAD_FILECONTENT_RELY
{
    char m_nType;
    int m_lUserId;
    int m_lFileId;
    int m_lPosition;
    int stage;
};

//删除文件请求包
struct STRU_PROTOCOL_DELETE_FILE_RQ
{
    char m_nType;
    int m_lUserId;
    char m_szFileName[MAXSIZE];
};


//删除文件回复包
struct STRU_PROTOCOL_DELETE_FILE_RS
{
    char m_nType;
    bool result;
};

#define FILE_PATH "D:\\disk"

#define DEBUG 0

#endif // PACKDEF_H
