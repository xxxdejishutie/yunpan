#include "TCPkernel.h"
TCPkernel* TCPkernel::m_tcpkernel = new TCPkernel; //����ģʽ ehanģʽ
TCPkernel::TCPkernel()
{
	m_pinet = new CTCPNet(this);
	m_pmysql = new CMySql;
	strcpy_s(m_path, FILE_PATH);
	CreateDirectoryA(FILE_PATH, 0);//�������򣬴����ļ���
}


TCPkernel::~TCPkernel()
{
	delete m_pinet;
	delete m_pmysql;
}


//��ʼ��mysql������
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


//�ر����ݿ������
void TCPkernel::Closesqlandnet()
{
	m_pinet->UnInitNetWork();
	m_pmysql->DisConnect();

}


//ע���¼��
void TCPkernel::do_REGISTER_RQ(char* buf,SOCKET sock)
{
	//�������ת���ɽṹ��
	STRU_REGISTER_RQ* p = (STRU_REGISTER_RQ*)buf;

	//����ظ���
	STRU_REGISTER_RS rs;
	rs.m_nType = _DEF_PROTOCOL_REGISTER_RS;
	rs.m_szResult = _register_failed;

	char kongjianbuf[Maxpath] = { 0 };//�������ƴ�ӿռ�ĵ�ַ
	//1.д�����ݿ�

	char scbuf[Maxsocle] = { 0 };
	sprintf_s(scbuf, "insert into User (username  ,password ,email) values('%s','%s',%lld);",
		p->m_szName, p->m_szPassword, p->m_ltel);//ƴ�����ݿ����

	//����mysql���ݿ�
	if (m_pmysql->UpdateMySql(scbuf))
	{   //�ɹ�
		rs.m_szResult = _register_success;
		//����ռ�
		sprintf_s(kongjianbuf,"%s\\file%lld", m_path, p->m_ltel);
		//��ÿ�����ֻ��Ŵ����ռ�
		cout << kongjianbuf << endl;
		CreateDirectoryA(kongjianbuf, 0);//·��,��ȫ����
	}

	m_pinet->SendData(sock, (char*)&rs, sizeof(rs));//���ͻظ���Ϣ
}

//�����¼�����
void TCPkernel::do_LOGIN_RQ(char* buf, SOCKET sock)
{
	STRU_LOGIN_RQ* rq = (STRU_LOGIN_RQ*)buf;
	list<string> lis;
	//У�������Ƿ���ȷ

	//��¼�ظ���
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
				//�ظ�
				rs.m_luserid = atoi(strmId.c_str());
				rs.m_szResult = _login_success;
			}
		}
	}

	m_pinet->SendData(sock, (char*)&rs, sizeof(rs));

}

//�����ȡ�ļ��б��
void TCPkernel::do_GETFILELIST_RQ(char* buf, SOCKET sock)
{
	STRU_GETFILELIST_RQ* rq = (STRU_GETFILELIST_RQ*)buf;
	list<string> lis;
	//��ѯ���id�µ������ļ�
	STRU_GETFILELIST_RS rs;
	rs.m_nType = _DEF_PROTOCOL_GETFILELIST_RS;

	char scbuf[Maxsocle] = { 0 };
	sprintf_s(scbuf, "select Filename,filesize,Updatetime from FileIndex where userID = %d;", rq->m_luserid);

	if(DEBUG)
		cout << scbuf << endl;

	m_pmysql->SelectMySql(scbuf, 3, lis);
	int i = 0;
	//���ļ���Ϣ��ȡ�����������͵��ͻ���
	while (lis.size() > 0)
	{
		string strf_name = lis.front();
		lis.pop_front();
		string strf_size = lis.front();
		lis.pop_front();
		string strf_time = lis.front();
		lis.pop_front();

		rs.m_aryFile[i].m_fileSize =atoi( strf_size.c_str());
		//д��ظ���
		strcpy_s(rs.m_aryFile[i].m_szFileName, strf_name.c_str());
		strcpy_s(rs.m_aryFile[i].m_szDateTime, strf_time.c_str());

		i++;
		//�ظ������ˣ�����������Ϣ����д���ˣ�����
		if (i == MAXFILENUM || lis.size() == 0)
		{
			rs.m_nFileNum = i;
			m_pinet->SendData(sock, (char*)&rs, sizeof(rs));
			i %= MAXFILENUM;
		}


	}

}



//�����ϴ��ļ���Ϣ�������ͻ��˵���ϴ��������õ�һ����
void TCPkernel::do_UPLOAD_FILEINFO(char* buf, SOCKET sock)
{
	STRU_UPLOAD_FILEINFO_RQ* rq = (STRU_UPLOAD_FILEINFO_RQ*)buf;
	STRU_UPLOAD_FILEINFO_RS rs;
	rs.m_lFileId = rq->m_lUserId;
	strcpy_s(rs.m_szFileMD5, rq->m_szFileMD5);
	rs.m_nType = _DEF_PROTOCOL_UPLOAD_FILEINFO_RS;
	rs.m_lPosition = 0;
	
	list<string> lisstr;
	//����������������ϴ��У��ϴ����
	char strsql[MAXFILENUM] = { 0 };

	sprintf_s(strsql, "select UserID,FileID,count from FileIndex where md5 ='%s' and filename = '%s';",rq->m_szFileMD5, rq->m_szFileName);
	m_pmysql->SelectMySql(strsql, 3, lisstr);
	if (lisstr.size() > 0)//�����㣬˵������ļ����ڣ��ϴ����������ϴ���
	{
		string user_id = lisstr.front();
		lisstr.pop_front();
		string file_id = lisstr.front();
		lisstr.pop_front();
		string f_count = lisstr.front();
		lisstr.pop_front();

		//Ϊ�ָ���f_id��ֵ
		rs.m_lFileId = _atoi64(file_id.c_str());

		long long user_m_id = _atoi64(user_id.c_str());
		//long long user_file_id = _atoi64(file_id.c_str());
		long long user_m_count = _atoi64(f_count.c_str());
		if (user_m_id == rq->m_lUserId)
		{
			rs.bresult = _upload_file_exists;
			//������ _upload_file_exists 0���Լ��������͹�������ˣ�����Ҫ�ٴ���
			//û����_upload_file_continue
			//rs.bresult = _upload_file_continue;

			//�������ļ���δ������ļ��б�����Ƕϵ�����
			uploadfileinfo* pupfileinfo = m_mapfileiftofileinfo[rs.m_lFileId];
			if (pupfileinfo)
			{
				rs.m_lPosition = pupfileinfo->fileposition;
				rs.bresult = _upload_file_continue;
			}


			
		}
		else
		{
			//�봫
			rs.bresult = _upload_file_success;
			//���ļ������û�

			sprintf_s(strsql, "insert into uandf(FileID,UserID) value('%s',%d);", file_id.c_str(), rq->m_lUserId);
			m_pmysql->UpdateMySql(strsql);
			//������һ
			sprintf_s(strsql, "update file set count = %lld where FileID = '%s';", user_m_count + 1, file_id.c_str());
			
			//�����ļ�������
			//7.17  15.29 ��δʵ�֣�  ������/����


			m_pmysql->UpdateMySql(strsql);

		}
	}
	else
	{
		//������
		rs.bresult = _upload_file_normal;
		//�����ֻ��ţ����ֻ��������ļ���
		sprintf_s(strsql, "select email from user where userid = %d;", rq->m_lUserId);
		m_pmysql->SelectMySql(strsql,1,lisstr);
		
		if (lisstr.size() > 0)//���û����ֻ��Ŵ���
		{
			string tel = lisstr.front();
			lisstr.pop_front();
			char pathtel[MAXFILENUM] = { 0 };
			
			//ƴ�ӵ�ַ
			sprintf_s(pathtel, "%s\\file%s\\%s", m_path, tel.c_str(), rq->m_szFileName);
			
			//���ƴ�Ӻ���ļ���ַ
			if(DEBUG)
				cout << "pathel = " << pathtel << endl;

			//���ļ����뵽�ļ��б����ü���Ϊ0
			sprintf_s(strsql, "insert into file(Filename,FileSize,FilePath,md5,count,CreationTime) value('%s',%d,'%s','%s',0,now());",
				rq->m_szFileName,rq->m_lFileSize, pathtel , rq->m_szFileMD5);
			m_pmysql->UpdateMySql(strsql);


			if(DEBUG)
				cout << strsql << endl;

			//��ѯ�ļ�id
			sprintf_s(strsql, "select fileid from file where filename = '%s' and md5 = '%s';",rq->m_szFileName,rq->m_szFileMD5);
			m_pmysql->SelectMySql(strsql, 1, lisstr);
			if(DEBUG)
				cout << strsql << endl;

			if (lisstr.size() > 0)
			{
				//Ϊ�ظ���f_id��ֵ
				rs.m_lFileId = _atoi64(lisstr.front().c_str());
				//��ȡ�ļ�id�����û����ļ�������һ��
				sprintf_s(strsql, "insert into UserFile(fileid,userid) value('%s',%d);", lisstr.front().c_str(), rq->m_lUserId);

				if(DEBUG)
					cout << strsql << endl;

				m_pmysql->UpdateMySql(strsql);
			}
			


			//�����ļ���Ϣ���ļ�id���ļ���С���ļ�λ��
			uploadfileinfo* pupfileinfo = new uploadfileinfo;
			pupfileinfo->filesize = rq->m_lFileSize;//�ļ���С
			pupfileinfo->user_id = rq->m_lUserId;//�û�id
			pupfileinfo->fileposition = 0;//�ļ��Ѵ���λ��
			pupfileinfo->file_id = rs.m_lFileId;//�ļ�id
			strcpy_s(pupfileinfo->pfilepath, pathtel);//�ļ���ַ
			pupfileinfo->of = new ofstream(pupfileinfo->pfilepath, ios::out | ios::binary);//�ļ�������
			m_mapfileiftofileinfo[rs.m_lFileId] = pupfileinfo;//���ļ�idӳ���ļ�������
		}
	}

	
	m_pinet->SendData(sock, (char*)&rs, sizeof(rs));
	//���˴������봫
	//���ļ���¼�����ݿ��У���ʼ�ϴ�

}


// ͨ��stat�ṹ�� ����ļ���С����λ�ֽ�
int getFileSize1(const char* fileName) {

	if (fileName == NULL) {
		return 0;
	}

	// ����һ���洢�ļ�(��)��Ϣ�Ľṹ�壬�������ļ���С�ʹ���ʱ�䡢����ʱ�䡢�޸�ʱ���
	struct stat statbuf;

	// �ṩ�ļ����ַ���������ļ����Խṹ��
	stat(fileName, &statbuf);

	// ��ȡ�ļ���С
	size_t filesize = statbuf.st_size;

	return filesize;
}


//�����ϴ��ļ����ݵð�
void TCPkernel::do_UPLOAD_FILECONTENT_RQ(char* buf, SOCKET sock)
{
	STRU_UPLOAD_FILECONTENT_RQ* rq = (STRU_UPLOAD_FILECONTENT_RQ*)buf;
	uploadfileinfo* pupfile = m_mapfileiftofileinfo[rq->m_lFileId];


	//5.28 ���ļ�д���������
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
		//���ļ�����д���ļ�

		//4.17 �ļ�д��ʱ����ƫ�Ƶ�ָ��λ��
		// 5.28 ����ƫ��
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
		//cout << "��ǰ�ļ���С�� " << a << endl;
		int fd = rq->m_lFileId;
		if (pupfile->fileposition == pupfile->filesize)
		{

			//�ļ��������
			pupfile->of->close();
			delete pupfile->of;
			pupfile->of = nullptr;
			int a = getFileSize1(pupfile->pfilepath);

			cout << "����ʱ�ļ���С �� " << a << endl;
			//������ڵ�ɾ��
			
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


			//�ļ����ü������һ
			sprintf_s(strsql, "update file set count = %d where f_id = '%d';", 1, fd);
			cout << strsql << endl;
			m_pmysql->UpdateMySql(strsql);
			delete pupfile;
			pupfile = NULL;

		}

	}

	//of.close();
}



//�ļ�����ģ��
	//�����ļ�����
void TCPkernel::do_DOWNLOAD_FILEINFO_RQ(char* buf, SOCKET sock)
{
	//ͨ���ļ������û�id��ѯ�ļ�id���ļ���ַ
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
		// �жϽṹ���ڲ��Ƿ����
		//���ڣ�Ϊ�ϵ����������ļ��ṹ���ڲ���λ�ü���
		//�����ڣ����뵽�����ļ��ṹ��
		lisstr.pop_front();
		//7.17 �޸ġ�������������������������������������������������������������������
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
	//��ͻ��˷����ļ��������ػظ���
	rs.m_lUserId = rq->m_lUserId;
	m_pinet->SendData(sock, (char*)&rs, sizeof(rs));
}





//�����ļ�����
void TCPkernel::DOWNLOAD_FILECONTENT(char* buf, SOCKET sock)
{
	//���ļ�
	uploadfileinfo* pupfileinfo = (uploadfileinfo*)buf;
	ifstream ifs;
	ifs.open(pupfileinfo->pfilepath, ios::in | ios::binary);
	//׷�Ӷ�ȡ��Ϣ
	ifs.seekg(pupfileinfo->fileposition,ios::beg);

	int pos = ifs.tellg();
	STRU_DOWNLOAD_FILECONTENT_RQ rq;
	rq.m_nType = _DEF_PROTOCOL_DOWNLOAD_FILECONTENT_SEND;
	rq.m_lUserId = pupfileinfo->user_id;
	rq.m_lFileId = pupfileinfo->file_id;
	rq.m_nNum = ifs.read(rq.m_szFileContent, MAXFILECONTENT).gcount();
	
	//������Ϣ
	m_pinet->SendData(sock, (char*)&rq, sizeof(rq));
}

//�ļ�����ȷ��
void TCPkernel::do_DOWNLOAD_FILECONTENT_RELY(char* buf, SOCKET sock)
{
	//�������Կͻ���ȷ�ϣ��Ѿ����յ�����Ϣ
	STRU_DOWNLOAD_FILECONTENT_RELY* rely = (STRU_DOWNLOAD_FILECONTENT_RELY*)buf;
	//�����ļ��ṹ�壬�ļ��ѷ���λ��
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


	//�ͻ����Ѿ�ȷ���յ��������򣬽��������ڷ���
	if (rely->m_lPosition == m_mapfileinfoinstall[air]->filesize)
	{
		m_mapfileinfoinstall.erase(air);
		return;
	}

	//�鿴״̬������Ǽ������䣬�ͼ�������
	if (rely->stage == _install_continue)
	{
		DOWNLOAD_FILECONTENT((char*)m_mapfileinfoinstall[air], sock);
	}
	
}



//ɾ���ļ�������
void TCPkernel::do_DELETE_FILE_RQ(char* buf, SOCKET sock)
{
	//�����û�����ļ��Ƿ���ڣ��������ļ����ü�����һ��
	//ɾ���û�����ļ���ӳ��
	STRU_PROTOCOL_DELETE_FILE_RQ* rq = (STRU_PROTOCOL_DELETE_FILE_RQ*)buf;
	char strsql[1024] = { 0 };
	STRU_PROTOCOL_DELETE_FILE_RS rs;
	rs.m_nType = _DEF_PROTOCOL_DELETE_FILE_RS;
	rs.result = _delete_faild;

	list<string> lisstr;
	//��ѯ����ļ��Ƿ����
	sprintf_s(strsql, "select fileid,count from fileindex where userid ='%d' and filename = '%s';", rq->m_lUserId, rq->m_szFileName);
	m_pmysql->SelectMySql(strsql, 2, lisstr);
	if (lisstr.size() > 1)
	{
		int f_id = (int)_atoi64(lisstr.front().c_str());
		lisstr.pop_front();
		int f_count = (int)_atoi64(lisstr.front().c_str());
		//�ļ����ü�����һ
		sprintf_s(strsql,"update file set count = %d where fileid = '%d';",f_count - 1,f_id);
		if(m_pmysql->UpdateMySql(strsql))
		{
			rs.result = _delete_success;
		}
		//ɾ���û�������ļ���ӳ��
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
			//9.23 �����̳߳ش���ѭ�����У��������飬����function��bind��װ

			return;
		}
		if (m_pprotomap[i].n_type == 0 && m_pprotomap[i].fun == 0)
			break;
		i++;
	}
}
