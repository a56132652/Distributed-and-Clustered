#ifndef _doyou_io_DBManager_HPP_
#define _doyou_io_DBManager_HPP_

#include"Log.hpp"
#include"CppSQLite3.h"

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
			//查询之后返回是否存在
			bool hasByKV(const char* table, const char* k, const char* v)
			{
				char sql_buff[1024] = {};
				auto sql = "SELECT 1 FROM %s WHERE %s='%s' LIMIT 1;";
				sprintf(sql_buff, sql, table, k, v);

				try
				{
					CppSQLite3Query query = _db.execQuery(sql_buff);
					return !query.eof();
				}
				catch (CppSQLite3Exception& e)
				{
					CELLLog_Error("DBManager::hasByKV(%s) error: %s", _db_name.c_str(), e.errorMessage());
				}
				return false;
			}
			//查询并返回json格式数据
			bool findByKV(const char* table, const char* k, const char* v, neb::CJsonObject& json)
			{
				char sql_buff[1024] = {};
				auto sql = "SELECT * FROM %s WHERE %s='%s';";
				sprintf(sql_buff, sql, table, k, v);

				try
				{
					//查询结果
					CppSQLite3Query query = _db.execQuery(sql_buff);
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
					CELLLog_Error("DBManager::findByKV(%s) error: %s", _db_name.c_str(), e.errorMessage());
					return false;
				}
				return true;
			}
		};
	}
}
#endif // !_doyou_io_DBManager_HPP_
