#ifndef _doyou_io_DBManager_HPP_
#define _doyou_io_DBManager_HPP_

#include"Log.hpp"
#include"CppSQLite3.h"
#include<sstream>
namespace doyou {
	namespace io {
		class DBManager
		{
		protected:
			CppSQLite3DB _db;
			std::string _db_name;
		public:
			bool open(const char* db_name)
			{
				_db_name = db_name;
				try
				{
					_db.open(db_name);
					return true;
				}
				catch (CppSQLite3Exception& e)
				{
					CELLLog_Error("DBManager::open(%s) error: %s", db_name, e.errorMessage());
				}
				return false;
			}

			bool close()
			{
				try
				{
					_db.close();
					return true;
				}
				catch (CppSQLite3Exception& e)
				{
					CELLLog_Error("DBManager::close(%s) error: %s", _db_name.c_str(), e.errorMessage());
				}
				return false;
			}
			//表是否存在
			bool tableExists(const char* szTable)
			{
				try
				{
					return _db.tableExists(szTable);
				}
				catch (CppSQLite3Exception& e)
				{
					CELLLog_Error("DBManager::tableExists(%s.%s) error: %s", _db_name.c_str(), szTable, e.errorMessage());
				}
				return false;
			}
			//查询结果转化为json格式数据
			bool query2json(CppSQLite3Query& query, neb::CJsonObject& json)
			{
				try
				{
					while (!query.eof())
					{
						neb::CJsonObject row;
						//返回当前表里有多少个字段
						int num = query.numFields();
						for (int n = 0; n < num; n++)
						{
							//取到相应字段名
							auto k = query.fieldName(n);
							//字段类型
							auto kType = query.fieldDataType(n);
							//整数
							if (SQLITE_INTEGER == kType)
							{
								int64 v = query.getInt64Field(k, 0);
								row.Add(k, v);
							}
							//文本
							else if (SQLITE_TEXT == kType)
							{
								auto v = query.getStringField(k);
								row.Add(k, v);
							}
							//浮点数
							else if (SQLITE_FLOAT == kType)
							{
								auto v = query.getFloatField(k);
								row.Add(k, v);
							}
							//二进制数据（图像，音视频）
							else if (SQLITE_BLOB == kType)
							{
								//int nLen = 0;
								//auto v = query.getBlobField(k, nLen);
								//将数据拷贝到由我们控制的BLOB数据内存中管理
								//row.Add新的BLOB数据地址
								//row.Add(k, (uint64)v);
							}
							else if (SQLITE_NULL == kType)
							{
								//row.Add(k, "NULL"); 
							}
						}
						json.Add(row);
						query.nextRow();
					}
				}
				catch (CppSQLite3Exception& e)
				{
					CELLLog_Error("DBManager::query2json(%s) error: %s", _db_name.c_str(), e.errorMessage());
					return false;
				}
				return true;
			}
			//执行查询语句，需要json结果
			bool execQuery(const char* sql, neb::CJsonObject& json)
			{
				CppSQLite3Query query;
				try
				{
					query = _db.execQuery(sql);
				}
				catch (CppSQLite3Exception& e)
				{
					CELLLog_Error("DBManager::execQuery(%s) sql:%s error: %s", _db_name.c_str(), sql, e.errorMessage());
					return false;
				}
				return query2json(query, json);
			}
			//执行查询语句,不需要结果
			bool execQuery(const char* sql)
			{
				CppSQLite3Query query;
				try
				{
					query = _db.execQuery(sql);
					return !query.eof();
				}
				catch (CppSQLite3Exception& e)
				{
					CELLLog_Error("DBManager::execQuery(%s) sql:%s error: %s", _db_name.c_str(), sql, e.errorMessage());
				}
				return false;
			}
			//执行DML语句
			int execDML(const char* sql)
			{
				try
				{
					return _db.execDML(sql);
				}
				catch (CppSQLite3Exception& e)
				{
					CELLLog_Error("DBManager::execDML(%s) sql:%s error: %s", _db_name.c_str(), sql, e.errorMessage());
				}
				return -1;
			}
			//查询之后返回是否存在
			template<typename vT>
			bool hasByKV(const char* table, const char* k, vT v)
			{//sql = "SELECT 1 FROM table WHERE k=v LIMIT 1;"
				std::stringstream ss;
				//使用字符串组织数据
				ss << "SELECT 1 FROM " << table << " WHERE " << k << '=';
				//typeid():返回指针或引用所指对象的实际类型
				if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
					ss << '\'' << v << '\'';
				else
					ss << v;

				ss << " LIMIT 1;";

				return execQuery(ss.str().c_str());
			}

			template<typename vT>
			bool findByKV(const char* table, const char* k, vT v, neb::CJsonObject& json)
			{//sql = "SELECT * FROM table WHERE k=v;"
				std::stringstream ss;
				ss << "SELECT * FROM " << table << " WHERE " << k << '=';
				//
				if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
					ss << '\'' << v << '\'';
				else
					ss << v;
				//
				return execQuery(ss.str().c_str(), json);
			}

			template<typename vT, typename v2T>
			bool findByKV2(const char* table, const char* k, vT v, const char* k2, v2T v2, neb::CJsonObject& json)
			{//sql = "SELECT * FROM table WHERE k=v and k2=v2;"
				std::stringstream ss;
				ss << "SELECT * FROM " << table << " WHERE " << k << '=';
				//
				if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
					ss << '\'' << v << '\'';
				else
					ss << v;
				//
				ss << " AND " << k2 << '=';
				if (typeid(v2) == typeid(const char*) || typeid(v2) == typeid(char*))
					ss << '\'' << v2 << '\'';
				else
					ss << v2;
				//
				return execQuery(ss.str().c_str(), json);
			}

			template<typename vT, typename uvT>
			int updateByKV(const char* table, const char* k, vT v, const char* uk, uvT uv)
			{//sql = "UPDATE table SET uk=uv WHERE k='v';"
				std::stringstream ss;
				ss << "UPDATE " << table << " SET " << uk << '=';
				//
				if (typeid(uv) == typeid(const char*) || typeid(uv) == typeid(char*))
					ss << '\'' << uv << '\'';
				else
					ss << uv;
				//
				ss << " WHERE " << k << '=';
				//
				if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
					ss << '\'' << v << '\'';
				else
					ss << v;
				//
				return execDML(ss.str().c_str());
			}

			template<typename vT, typename v2T, typename uvT>
			int updateByKV2(const char* table, const char* k, vT v, const char* k2, v2T v2, const char* uk, uvT uv)
			{//sql = "UPDATE table SET uk=uv WHERE k='v' AND k2='v2';"
				std::stringstream ss;
				ss << "UPDATE " << table << " SET " << uk << '=';
				//
				if (typeid(uv) == typeid(const char*) || typeid(uv) == typeid(char*))
					ss << '\'' << uv << '\'';
				else
					ss << uv;
				//
				ss << " WHERE " << k << '=';
				//
				if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
					ss << '\'' << v << '\'';
				else
					ss << v;
				//
				ss << " AND " << k2 << '=';
				if (typeid(v2) == typeid(const char*) || typeid(v2) == typeid(char*))
					ss << '\'' << v2 << '\'';
				else
					ss << v2;
				//
				return execDML(ss.str().c_str());
			}

			template<typename vT>
			int deleteByKV(const char* table, const char* k, vT v)
			{//sql = "DELETE FROM table WHERE k=v;"
				std::stringstream ss;
				ss << "DELETE FROM " << table << " WHERE " << k << '=';
				//
				if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
					ss << '\'' << v << '\'';
				else
					ss << v;
				//
				return execDML(ss.str().c_str());
			}

			template<typename vT, typename v2T>
			int deleteByKV2(const char* table, const char* k, vT v, const char* k2, v2T v2)
			{//sql = "DELETE FROM table WHERE k=v and k2=v2;"
				std::stringstream ss;
				ss << "DELETE FROM " << table << " WHERE " << k << '=';
				//
				if (typeid(v) == typeid(const char*) || typeid(v) == typeid(char*))
					ss << '\'' << v << '\'';
				else
					ss << v;
				//
				ss << " AND " << k2 << '=';
				if (typeid(v2) == typeid(const char*) || typeid(v2) == typeid(char*))
					ss << '\'' << v2 << '\'';
				else
					ss << v2;
				//
				return execDML(ss.str().c_str());
			}
		};
	}
}
#endif // !_doyou_io_DBManager_HPP_
