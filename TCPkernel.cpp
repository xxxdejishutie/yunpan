#include "TCPkernel.h"
TCPkernel* TCPkernel::m_tcpkernel = new TCPkernel; //单例模式 ehan模式
TCPkernel::TCPkernel()
{
	m_pinet = new CTCPNet(this);
	m_pmysql = new CMySql;
	strcpy_s(m_path, FILE_PATH);
	CreateDirectoryA(FILE_PATH, 0);//不存在则，创建文件夹
}


TCPkernel::~TCPkernel()
{
	delete m_pinet;
	delete m_pmysql;
}


//初始化mysql和网络
bool TCPkernel::Opensqlandnet()
{
	if (!m_pinet->InitNetWork())
	{
		cout << "net fail" << endl;
	}
	if (!m_pmysql->ConnectMySql("127.0.0.1","root","123456","jwgl"))
	{
		cout << "mysql fail" << endl;
	}
	return 1;
}


//关闭数据库和网络
void TCPkernel::Closesqlandnet()
{
	m_pinet->UnInitNetWork();
	m_pmysql->DisConnect();

}


//注册登录包
void TCPkernel::do_REGISTER_RQ(char* buf,SOCKET sock)
{
	//请求包，转化成结构体
	STRU_REGISTER_RQ* p = (STRU_REGISTER_RQ*)buf;

	//请求回复包
	STRU_REGISTER_RS rs;
	rs.m_nType = _DEF_PROTOCOL_REGISTER_RS;
	rs.m_szResult = _register_failed;

	char kongjianbuf[Maxpath] = { 0 };//用来存放拼接空间的地址
	//1.写入数据库

	char scbuf[Maxsocle] = { 0 };
	sprintf_s(scbuf, "insert into User (username  ,password ,email) values('%s','%s',%lld);",
		p->m_szName, p->m_szPassword, p->m_ltel);//拼接数据库语句

	//更新mysql数据库
	if (m_pmysql->UpdateMySql(scbuf))
	{   //成功
		rs.m_szResult = _register_success;
		//分配空间
		sprintf_s(kongjianbuf,"%s\\file%lld", m_path, p->m_ltel);
		//按每个人手机号创建空间
		cout << kongjianbuf << endl;
		CreateDirectoryA(kongjianbuf, 0);//路径,安全属性
	}

	m_pinet->SendData(sock, (char*)&rs, sizeof(rs));//发送回复信息
}

//处理登录请求包
void TCPkernel::do_LOGIN_RQ(char* buf, SOCKET sock)
{
	STRU_LOGIN_RQ* rq = (STRU_LOGIN_RQ*)buf;
	list<string> lis;
	//校验密码是否正确

	//登录回复包
	STRU_LOGIN_RS rs;
	rs.m_nType = _DEF_PROTOCOL_LOGIN_RS;
	rs.m_szResult = _login_user_noexist;
	
	char scbuf[Maxsocle] = { 0 };
	sprintf_s(scbuf, "select userid  ,password  from User  where username = '%s';",rq->m_szName);

	
	if (m_pmysql->SelectMySql(scbuf, 2, lis))
	{
		if (lis.size() > 0)
		{
			rs.m_szResult = _login_password_err;
			string strmId = lis.front();
			lis.pop_front();
			string password = lis.front();
			lis.pop_front();
			if (0 == strcmp(password.c_str(), rq->m_szPassword))
			{
				//回复
				rs.m_luserid = atoi(strmId.c_str());
				rs.m_szResult = _login_success;
			}
		}
	}

	m_pinet->SendData(sock, (char*)&rs, sizeof(rs));

}

//处理获取文件列表包
void TCPkernel::do_GETFILELIST_RQ(char* buf, SOCKET sock)
{
	STRU_GETFILELIST_RQ* rq = (STRU_GETFILELIST_RQ*)buf;
	list<string> lis;
	//查询这个id下的所有文件
	STRU_GETFILELIST_RS rs;
	rs.m_nType = _DEF_PROTOCOL_GETFILELIST_RS;

	char scbuf[Maxsocle] = { 0 };
	sprintf_s(scbuf, "select Filename,filesize,Updatetime from FileIndex where userID = %d;", rq->m_luserid);

	if(DEBUG)
		cout << scbuf << endl;

	m_pmysql->SelectMySql(scbuf, 3, lis);
	int i = 0;
	//将文件信息提取出来，并发送到客户端
	while (lis.size() > 0)
	{
		string strf_name = lis.front();
		lis.pop_front();
		string strf_size = lis.front();
		lis.pop_front();
		string strf_time = lis.front();
		lis.pop_front();

		rs.m_aryFile[i].m_fileSize =atoi( strf_size.c_str());
		//写入回复包
		strcpy_s(rs.m_aryFile[i].m_szFileName, strf_name.c_str());
		strcpy_s(rs.m_aryFile[i].m_szDateTime, strf_time.c_str());

		i++;
		//回复包满了，或者所有信息都被写入了，发送
		if (i == MAXFILENUM || lis.size() == 0)
		{
			rs.m_nFileNum = i;
			m_pinet->SendData(sock, (char*)&rs, sizeof(rs));
			i %= MAXFILENUM;
		}


	}

}



//处理上传文件信息包，即客户端点击上传后发送来得第一个包
void TCPkernel::do_UPLOAD_FILEINFO(char* buf, SOCKET sock)
{
	STRU_UPLOAD_FILEINFO_RQ* rq = (STRU_UPLOAD_FILEINFO_RQ*)buf;
	STRU_UPLOAD_FILEINFO_RS rs;
	rs.m_lFileId = rq->m_lUserId;
	strcpy_s(rs.m_szFileMD5, rq->m_szFileMD5);
	rs.m_nType = _DEF_PROTOCOL_UPLOAD_FILEINFO_RS;
	rs.m_lPosition = 0;
	
	list<string> lisstr;
	//三种情况，传过，上传中，上传完毕
	char strsql[MAXFILENUM] = { 0 };

	sprintf_s(strsql, "select UserID,FileID,count from FileIndex where md5 ='%s' and filename = '%s';",rq->m_szFileMD5, rq->m_szFileName);
	m_pmysql->SelectMySql(strsql, 3, lisstr);
	if (lisstr.size() > 0)//大于零，说明这个文件存在，上传过，或者上传中
	{
		string user_id = lisstr.front();
		lisstr.pop_front();
		string file_id = lisstr.front();
		lisstr.pop_front();
		string f_count = lisstr.front();
		lisstr.pop_front();

		//为恢复包f_id赋值
		rs.m_lFileId = _atoi64(file_id.c_str());

		long long user_m_id = _atoi64(user_id.c_str());
		//long long user_file_id = _atoi64(file_id.c_str());
		long long user_m_count = _atoi64(f_count.c_str());
		if (user_m_id == rq->m_lUserId)
		{
			rs.bresult = _upload_file_exists;
			//传完了 _upload_file_exists 0，自己曾经传送过，完成了，不需要再传了
			//没传完_upload_file_continue
			//rs.bresult = _upload_file_continue;

			//如果这个文件在未传完的文件列表里，就是断点续传
			uploadfileinfo* pupfileinfo = m_mapfileiftofileinfo[rs.m_lFileId];
			if (pupfileinfo)
			{
				rs.m_lPosition = pupfileinfo->fileposition;
				rs.bresult = _upload_file_continue;
			}


			
		}
		else
		{
			//秒传
			rs.bresult = _upload_file_success;
			//把文件给该用户

			sprintf_s(strsql, "insert into uandf(FileID,UserID) value('%s',%d);", file_id.c_str(), rq->m_lUserId);
			m_pmysql->UpdateMySql(strsql);
			//计数加一
			sprintf_s(strsql, "update file set count = %lld where FileID = '%s';", user_m_count + 1, file_id.c_str());
			
			//更新文件索引表
			//7.17  15.29 （未实现）  触发器/代码


			m_pmysql->UpdateMySql(strsql);

		}
	}
	else
	{
		//正常传
		rs.bresult = _upload_file_normal;
		//查找手机号，以手机号设置文件夹
		sprintf_s(strsql, "select email from user where userid = %d;", rq->m_lUserId);
		m_pmysql->SelectMySql(strsql,1,lisstr);
		
		if (lisstr.size() > 0)//该用户的手机号存在
		{
			string tel = lisstr.front();
			lisstr.pop_front();
			char pathtel[MAXFILENUM] = { 0 };
			
			//拼接地址
			sprintf_s(pathtel, "%s\\file%s\\%s", m_path, tel.c_str(), rq->m_szFileName);
			
			//输出拼接后得文件地址
			if(DEBUG)
				cout << "pathel = " << pathtel << endl;

			//将文件加入到文件列表，引用计数为0
			sprintf_s(strsql, "insert into file(Filename,FileSize,FilePath,md5,count,CreationTime) value('%s',%d,'%s','%s',0,now());",
				rq->m_szFileName,rq->m_lFileSize, pathtel , rq->m_szFileMD5);
			m_pmysql->UpdateMySql(strsql);


			if(DEBUG)
				cout << strsql << endl;

			//查询文件id
			sprintf_s(strsql, "select fileid from file where filename = '%s' and md5 = '%s';",rq->m_szFileName,rq->m_szFileMD5);
			m_pmysql->SelectMySql(strsql, 1, lisstr);
			if(DEBUG)
				cout << strsql << endl;

			if (lisstr.size() > 0)
			{
				//为回复包f_id赋值
				rs.m_lFileId = _atoi64(lisstr.front().c_str());
				//获取文件id，把用户和文件关联在一起
				sprintf_s(strsql, "insert into UserFile(fileid,userid) value('%s',%d);", lisstr.front().c_str(), rq->m_lUserId);

				if(DEBUG)
					cout << strsql << endl;

				m_pmysql->UpdateMySql(strsql);
			}
			


			//储存文件信息，文件id，文件大小，文件位置
			uploadfileinfo* pupfileinfo = new uploadfileinfo;
			pupfileinfo->filesize = rq->m_lFileSize;//文件大小
			pupfileinfo->user_id = rq->m_lUserId;//用户id
			pupfileinfo->fileposition = 0;//文件已传输位置
			pupfileinfo->file_id = rs.m_lFileId;//文件id
			strcpy_s(pupfileinfo->pfilepath, pathtel);//文件地址
			pupfileinfo->of = new ofstream(pupfileinfo->pfilepath, ios::out | ios::binary);//文件描述符
			m_mapfileiftofileinfo[rs.m_lFileId] = pupfileinfo;//用文件id映射文件描述符
		}
	}

	
	m_pinet->SendData(sock, (char*)&rs, sizeof(rs));
	//别人传过，秒传
	//将文件记录在数据库中，开始上传

}


// 通过stat结构体 获得文件大小，单位字节
int getFileSize1(const char* fileName) {

	if (fileName == NULL) {
		return 0;
	}

	// 这是一个存储文件(夹)信息的结构体，其中有文件大小和创建时间、访问时间、修改时间等
	struct stat statbuf;

	// 提供文件名字符串，获得文件属性结构体
	stat(fileName, &statbuf);

	// 获取文件大小
	size_t filesize = statbuf.st_size;

	return filesize;
}


//处理上传文件内容得包
void TCPkernel::do_UPLOAD_FILECONTENT_RQ(char* buf, SOCKET sock)
{
	STRU_UPLOAD_FILECONTENT_RQ* rq = (STRU_UPLOAD_FILECONTENT_RQ*)buf;
	uploadfileinfo* pupfile = m_mapfileiftofileinfo[rq->m_lFileId];


	//5.28 大文件写入问题更改
	/*if (!of) {
	  of = new ofstream(pupfile->pfilepath, ios::out | ios::binary);
	}*/


	if (!pupfile->of)
	{
		cout << "open file shibai" << endl;
		return;
	}

	if (rq->m_lUserId == pupfile->user_id && rq->m_lFileId == pupfile->file_id)
	{
		//将文件内容写入文件

		//4.17 文件写入时，先偏移到指定位置
		// 5.28 不再偏移
		if(DEBUG)
			cout << " file pos now is " << pupfile->of->tellp() << endl;
		
		//of.seekp(rq->m_pos);
		//cout << " file pos after seekp  is " << of->tellp() << endl;
		
		pupfile->fileposition = rq->m_pos;

		//  of << rq->m_szFileContent;

		pupfile->of->write(rq->m_szFileContent, rq->m_nNum);
		if(DEBUG)
			cout << " file pos after write is " << pupfile->of->tellp() << endl;
		
		pupfile->fileposition += rq->m_nNum;
		if (DEBUG)
			cout << "pupfile->fileposition is " << pupfile->fileposition;
		
		int a = getFileSize1(pupfile->pfilepath);
		//cout << "当前文件大小： " << a << endl;
		int fd = rq->m_lFileId;
		if (pupfile->fileposition == pupfile->filesize)
		{

			//文件传输完毕
			pupfile->of->close();
			delete pupfile->of;
			pupfile->of = nullptr;
			int a = getFileSize1(pupfile->pfilepath);

			cout << "结束时文件大小 ： " << a << endl;
			//将这个节点删除
			
			auto ite = m_mapfileiftofileinfo.begin();

			while (ite != m_mapfileiftofileinfo.end())
			{
				if ((*ite).second == pupfile)
				{
					m_mapfileiftofileinfo.erase(ite);
					break;
				}

				ite++;
			}
			char strsql[MAXFILENUM] = { 0 };


			//文件引用计数变成一
			sprintf_s(strsql, "update file set count = %d where f_id = '%d';", 1, fd);
			cout << strsql << endl;
			m_pmysql->UpdateMySql(strsql);
			delete pupfile;
			pupfile = NULL;

		}

	}

	//of.close();
}



//文件下载模块
	//下载文件请求
void TCPkernel::do_DOWNLOAD_FILEINFO_RQ(char* buf, SOCKET sock)
{
	//通过文件名和用户id查询文件id，文件地址
	STRU_DOWNLOAD_FILEINFO_RQ* rq = (STRU_DOWNLOAD_FILEINFO_RQ*)buf;
	STRU_DOWNLOAD_FILEINFO_RS rs;
	rs.bresult = true;
	rs.m_nType = _DEF_PROTOCOL_DOWNLOAD_FILEINFO_RS;
	strcpy_s(rs.m_szFileName, rq->m_szFileName);

	char strsql[1024] = { 0 };
	list<string> lisstr;
	sprintf_s(strsql, "select fileid from FileIndex where userid ='%d' and filename = '%s';", rq->m_lUserId, rq->m_szFileName);
	m_pmysql->SelectMySql(strsql, 1, lisstr);
	if (lisstr.size() >= 1)
	{

		rs.m_lFileId = (int)_atoi64(lisstr.front().c_str());
		// 判断结构体内部是否存在
		//存在，为断点续传，把文件结构体内部的位置加入
		//不存在，加入到下载文件结构体
		lisstr.pop_front();
		//7.17 修改——————————————————————————————————
		sprintf_s(strsql, "select FileSize from file where fileid = %d;", rs.m_lFileId,rq->m_lUserId);
		m_pmysql->SelectMySql(strsql, 1, lisstr);
		int filesize = (int)_atoi64(lisstr.front().c_str());
		rs.m_fisize = filesize;
		lisstr.pop_front();
		sprintf_s(strsql, "select email from user where userid = %d;", rq->m_lUserId);
		m_pmysql->SelectMySql(strsql, 1, lisstr);
		int phonenum = (int)_atoi64(lisstr.front().c_str());
		lisstr.pop_front();

		pair<int,int> air;
		air.first = rq->m_lUserId;
		air.second = rs.m_lFileId;
		uploadfileinfo* pupfileinfo = m_mapfileinfoinstall[air];
		if (pupfileinfo != NULL)
		{
			rs.m_lPosition = m_mapfileinfoinstall[air]->fileposition;
		}
		else
		{
			char path1[MAXFILENUM] = { 0 };
			sprintf_s(path1, "%s\\file%d\\%s", m_path, phonenum , rq->m_szFileName);

			pupfileinfo = new uploadfileinfo;
			pupfileinfo->filesize = filesize;
			pupfileinfo->user_id = rq->m_lUserId;
			pupfileinfo->fileposition = 0;
			pupfileinfo->file_id = rs.m_lFileId;
			strcpy_s(pupfileinfo->pfilepath, path1);
			m_mapfileinfoinstall[air] = pupfileinfo;

			rs.m_lPosition = 0;
		}
	}
	else
	{
		rs.bresult = false;
	}
	//向客户端发送文件可以下载回复包
	rs.m_lUserId = rq->m_lUserId;
	m_pinet->SendData(sock, (char*)&rs, sizeof(rs));
}





//下载文件发送
void TCPkernel::DOWNLOAD_FILECONTENT(char* buf, SOCKET sock)
{
	//打开文件
	uploadfileinfo* pupfileinfo = (uploadfileinfo*)buf;
	ifstream ifs;
	ifs.open(pupfileinfo->pfilepath, ios::in | ios::binary);
	//追加读取信息
	ifs.seekg(pupfileinfo->fileposition,ios::beg);

	int pos = ifs.tellg();
	STRU_DOWNLOAD_FILECONTENT_RQ rq;
	rq.m_nType = _DEF_PROTOCOL_DOWNLOAD_FILECONTENT_SEND;
	rq.m_lUserId = pupfileinfo->user_id;
	rq.m_lFileId = pupfileinfo->file_id;
	rq.m_nNum = ifs.read(rq.m_szFileContent, MAXFILECONTENT).gcount();
	
	//发送消息
	m_pinet->SendData(sock, (char*)&rq, sizeof(rq));
}

//文件接收确认
void TCPkernel::do_DOWNLOAD_FILECONTENT_RELY(char* buf, SOCKET sock)
{
	//接受来自客户的确认，已经接收到得消息
	STRU_DOWNLOAD_FILECONTENT_RELY* rely = (STRU_DOWNLOAD_FILECONTENT_RELY*)buf;
	//更改文件结构体，文件已发送位置
	pair<int, int> air;
	air.first = rely->m_lUserId;
	air.second = rely->m_lFileId;
	uploadfileinfo* pupfileinfo = m_mapfileinfoinstall[air];

	if (m_mapfileinfoinstall[air])
	{
		m_mapfileinfoinstall[air]->fileposition = rely->m_lPosition;
		if(DEBUG)
			cout<< "rely->m_lPosition is :" << rely->m_lPosition << endl;
	}


	//客户端已经确认收到完整程序，结束，不在发送
	if (rely->m_lPosition == m_mapfileinfoinstall[air]->filesize)
	{
		m_mapfileinfoinstall.erase(air);
		return;
	}

	//查看状态，如果是继续传输，就继续发送
	if (rely->stage == _install_continue)
	{
		DOWNLOAD_FILECONTENT((char*)m_mapfileinfoinstall[air], sock);
	}
	
}



//删除文件处理函数
void TCPkernel::do_DELETE_FILE_RQ(char* buf, SOCKET sock)
{
	//查找用户这个文件是否存在，存在则文件引用计数减一，
	//删除用户与该文件的映射
	STRU_PROTOCOL_DELETE_FILE_RQ* rq = (STRU_PROTOCOL_DELETE_FILE_RQ*)buf;
	char strsql[1024] = { 0 };
	STRU_PROTOCOL_DELETE_FILE_RS rs;
	rs.m_nType = _DEF_PROTOCOL_DELETE_FILE_RS;
	rs.result = _delete_faild;

	list<string> lisstr;
	//查询这个文件是否存在
	sprintf_s(strsql, "select fileid,count from fileindex where userid ='%d' and filename = '%s';", rq->m_lUserId, rq->m_szFileName);
	m_pmysql->SelectMySql(strsql, 2, lisstr);
	if (lisstr.size() > 1)
	{
		int f_id = (int)_atoi64(lisstr.front().c_str());
		lisstr.pop_front();
		int f_count = (int)_atoi64(lisstr.front().c_str());
		//文件引用计数减一
		sprintf_s(strsql,"update file set count = %d where fileid = '%d';",f_count - 1,f_id);
		if(m_pmysql->UpdateMySql(strsql))
		{
			rs.result = _delete_success;
		}
		//删除用户与这个文件的映射
		sprintf_s(strsql,"delete from userfile where userid = %d and fileid = %d; ", rq->m_lUserId, f_id);
		if (m_pmysql->UpdateMySql(strsql))
		{
			rs.result = _delete_success;
		}
	}

	m_pinet->SendData(sock, (char*)&rs, sizeof(rs));

}





protomap m_pprotomap[] = {
	{_DEF_PROTOCOL_REGISTER_RQ,&TCPkernel::do_REGISTER_RQ},
	{_DEF_PROTOCOL_LOGIN_RQ,&TCPkernel::do_LOGIN_RQ},
	{_DEF_PROTOCOL_GETFILELIST_RQ,&TCPkernel::do_GETFILELIST_RQ},
	{_DEF_PROTOCOL_UPLOAD_FILEINFO_RQ,&TCPkernel::do_UPLOAD_FILEINFO},
	{_DEF_PROTOCOL_UPLOAD_FILECONTENT_RQ,&TCPkernel::do_UPLOAD_FILECONTENT_RQ},
	{_DEF_PROTOCOL_DELETE_FILE_RQ,&TCPkernel::do_DELETE_FILE_RQ},
	{_DEF_PROTOCOL_DOWNLOAD_FILEINFO_RQ,&TCPkernel::do_DOWNLOAD_FILEINFO_RQ},
	//{_DEF_PROTOCOL_DOWNLOAD_FILECONTENT_SEND,&TCPkernel::do_DOWNLOAD_FILECONTENT},
	{_DEF_PROTOCOL_DOWNLOAD_FILECONTENT_RELY,&TCPkernel::do_DOWNLOAD_FILECONTENT_RELY},
	{0,0}

};

void TCPkernel::dealtext(char *buf,SOCKET sock)
{
	int i = 0;
	while (1)
	{
		if (m_pprotomap[i].n_type == *buf)
		{
			(this->*m_pprotomap[i].fun)(buf, sock);
			//9.23 加入线程池处理，循环队列，无锁数组，采用function和bind封装

			return;
		}
		if (m_pprotomap[i].n_type == 0 && m_pprotomap[i].fun == 0)
			break;
		i++;
	}
}
