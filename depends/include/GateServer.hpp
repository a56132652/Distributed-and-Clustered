#ifndef _doyou_io_GateServer_HPP_
#define _doyou_io_GateServer_HPP_

#include"INetServer.hpp"
#include"INetTransfer.hpp"

namespace doyou {
	namespace io {
		class GateServer
		{
		private:
			INetServer _netserver;
			INetTransfer _transfer;
		public:
			void Init()
			{
				_netserver.Init();
				//std::bind() c++11新方法 std::placeholders::_1占位符
				/*
				可将bind函数看作是一个通用的函数适配器，它接受一个可调用对象，生成一个新的可调用对象来“适应”原对象的参数列表。

				调用bind的一般形式：auto newCallable = bind(callable,arg_list);

				其中，newCallable本身是一个可调用对象，arg_list是一个逗号分隔的参数列表，对应给定的callable的参数。即，当我们调用newCallable时，newCallable会调用callable,并传给它arg_list中的参数。

				*/
				_netserver.on_other_msg = std::bind(&GateServer::on_other_msg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
				_netserver.on_client_leave = std::bind(&GateServer::on_client_leave, this, std::placeholders::_1);
				_netserver.reg_msg_call("cs_msg_heart", std::bind(&GateServer::cs_msg_heart, this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
				_netserver.reg_msg_call("ss_reg_api", std::bind(&GateServer::ss_reg_api, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
				
			}

			void Close()
			{
				_netserver.Close();
			}

		private:
			void cs_msg_heart(Server* server, INetClientS* client, neb::CJsonObject& msg)
			{
				CELLLog_Info("GateServer::cs_msg_heart");

				neb::CJsonObject ret;
				ret.Add("data", "wo ye bu ji dao.");
				client->response(msg, ret);

				//client->respone(msg, "wo ye bu ji dao.");
			}
			//响应LoginServer注册服务请求
			void ss_reg_api(Server* server, INetClientS* client, neb::CJsonObject& msg)
			{
				auto sskey = msg["data"]("sskey");
				auto sskey_local = Config::Instance().getStr("sskey", "ssmm00@123456");
				if (sskey != sskey_local)
				{
					neb::CJsonObject ret;
					//状态码 state 0-fail ,1-success
					ret.Add("state", 0);
					ret.Add("msg", "sskey error.");
					client->response(msg, ret);
					return;
				}
				//客户端类型
				auto type = msg["data"]("type");
				//客户端名称
				auto name = msg["data"]("name");

				client->link_type(type);
				client->link_name(name);
				client->is_ss_link(true);
				/*
					msg["data"]("type"):返回 "type" 字段所对应的字符串
					msg["data"]["apis"]:返回 "apis" 字段所对应的json结构，结构下又包含字符串
				*/
				auto apis = msg["data"]["apis"];

				if (!apis.IsArray())
				{
					neb::CJsonObject ret;
					ret.Add("state", 0);
					ret.Add("msg", "not found apis.");
					client->response(msg, ret);
					return;
				}
				int size = apis.GetArraySize();
				for (size_t i = 0; i < size; i++)
				{
					CELLLog_Info("ss_reg_api: %s >> %s", name.c_str(), apis(i).c_str());
					_transfer.add(apis(i), client);
				}
			}

			void on_other_msg(Server* server, INetClientS* client, std::string& cmd, neb::CJsonObject& msg)
			{
				auto str = msg.ToString();
				if (!_transfer.on_net_msg_do(cmd, str))
				{
					CELLLog_Info("on_other_msg: transfer not found cmd<%s>.", cmd.c_str());
				}
			}

			void on_client_leave(INetClientS* client)
			{
				if(client->is_ss_link())
					_transfer.del(client);
			}
		};
	}
}
#endif // !_doyou_io_GateServer_HPP_
