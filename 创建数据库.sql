
create database yunpan;
use yunpan;
CREATE TABLE fileinfo (
  f_id INT(11) NOT NULL AUTO_INCREMENT,
  f_name VARCHAR(45) NOT NULL,
  f_size VARCHAR(45) NOT NULL,
  f_path VARCHAR(45) NOT NULL,
  f_md5 VARCHAR(45) NOT NULL,
  f_count VARCHAR(45) NOT NULL,
  f_time  TIMESTAMP NOT NULL,
  PRIMARY KEY (f_id));
  
  
  CREATE TABLE userinfo (
  m_id INT NOT NULL AUTO_INCREMENT,
  m_name VARCHAR(45) NOT NULL,
  m_password VARCHAR(45) NOT NULL,
  m_phonenum VARCHAR(45) NOT NULL,
  PRIMARY KEY (m_id),
  UNIQUE INDEX m_phonenum_UNIQUE (m_phonenum ASC));
  
  
  CREATE TABLE uandf (
  f_id INT NOT NULL,
  m_id VARCHAR(45) NOT NULL,
  num INT NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (num));

  create view myview 
  as 
  select uandf.f_id ,uandf.m_id,f_name,m_phonenum,m_name,f_path,f_md5,f_time ,f_size,f_count
  from fileinfo,uandf,userinfo 
  where fileinfo.f_id = uandf.f_id and uandf.m_id = userinfo.m_id;
  
  set sql_safe_updates=0; 