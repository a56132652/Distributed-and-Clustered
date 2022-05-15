#ifndef _doyou_io_LoginServer_HPP_
#define _doyou_io_LoginServer_HPP_

#include"INetClient.hpp"

namespace doyou {
	namespace io {
		class LoginServer
		{
		private:
			INetClient _csGate;
		public:
			void Init()
			{
				_csGate.connect("csGate","ws://192.168.1.104:4567");
				//注册关注事件
				_csGate.reg_msg_call("onopen", std::bind(&LoginServer::onopen_csGate, this, std::placeholders::_1, std::placeholders::_2));
				
				_csGate.reg_msg_call("cs_msg_login", std::bind(&LoginServer::cs_msg_login, this, std::placeholders::_1, std::placeholders::_2));
			}

			void Run()
			{
				_csGate.run(1);
			}

			void Close()
			{
				_csGate.close();
			}

		private:
			//收到 onopen 消息时的回调，收到onopen消息后向网关服务器发起注册服务请求
			void onopen_csGate(INetClient* client, neb::CJsonObject& msg)
			{
				neb::CJsonObject json;
				//告知网关自己是什么类型的服务
				json.Add("type", "LoginServer");
				//自己的名字
				json.Add("name", "LoginServer001");
				//校验机制，网关服务器会校验该值
				json.Add("sskey", "ssmm00@123456");
				json.AddEmptySubArray("apis");
				/*
					告知网关自己关心什么类型的消息
					网关收到该类型的消息到会分发给自己
				*/
				//登录
				json["apis"].Add("cs_msg_login");
				//注册
				json["apis"].Add("cs_msg_register");
				//改密码
				json["apis"].Add("cs_msg_change_pw");
				client->request("ss_reg_api", json, [](INetClient* client, neb::CJsonObject& msg) {
					CELLLog_Info(msg("data").c_str());
				});
			}

			void cs_msg_login(INetClient* client, neb::CJsonObject& msg)
			{
				CELLLog_Info("LoginServer::cs_msg_login");

				neb::CJsonObject ret;
				ret.Add("data", "login successs.");
				client->response(msg, ret);
			}
		};
	}
}
#endif // !_doyou_io_LoginServer_HPP_
