#ifndef _doyou_io_INetClientS_HPP_
#define _doyou_io_INetClientS_HPP_

#include"WebSocketClientS.hpp"
#include"CJsonObject.hpp"

namespace doyou {
	namespace io {

		//客户端数据类型
		class INetClientS :public WebSocketClientS
		{
		private:
			//对客户端请求进行限流，对上册服务返回的消息不限流
			std::string _link_name;
			//连接类型(上层服务 or 客户端)，默认为客户端
			std::string _link_type = "client";
			//是否是服务，是服务的话不进行限流
			bool _is_ss_link = false;
		public:
			INetClientS(SOCKET sockfd = INVALID_SOCKET, int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE) :
				WebSocketClientS(sockfd, sendSize, recvSize)
			{

			}

			std::string& link_name()
			{
				return _link_name;
			}

			void link_name(std::string& str)
			{
				_link_name = str;
			}

			std::string& link_type()
			{
				return _link_type;
			}

			void link_type(std::string& str)
			{
				_link_type = str;
			}

			bool is_ss_link()
			{
				return _is_ss_link;
			}

			void is_ss_link(bool b)
			{
				_is_ss_link = b;
			}

			void response(int msgId, std::string data)
			{
				neb::CJsonObject ret;
				ret.Add("msgId", msgId);
				ret.Add("is_resp", true, true);
				ret.Add("time", Time::system_clock_now());
				ret.Add("data", data);

				std::string retStr = ret.ToString();
				this->writeText(retStr.c_str(), retStr.length());
			}

			void response(neb::CJsonObject& msg, std::string data)
			{
				int msgId = 0;
				if (!msg.Get("msgId", msgId))
				{
					CELLLog_Error("not found key<%s>.", "msgId");
					return;
				}

				neb::CJsonObject ret;
				ret.Add("msgId", msgId);
				ret.Add("is_resp", true, true);
				ret.Add("time", Time::system_clock_now());
				ret.Add("data", data);

				std::string retStr = ret.ToString();
				this->writeText(retStr.c_str(), retStr.length());
			}

			void response(neb::CJsonObject& msg, neb::CJsonObject& ret)
			{
				int msgId = 0;
				if (!msg.Get("msgId", msgId))
				{
					CELLLog_Error("not found key<%s>.", "msgId");
					return;
				}

				ret.Add("msgId", msgId);
				ret.Add("is_resp", true, true);
				ret.Add("time", Time::system_clock_now());

				std::string retStr = ret.ToString();
				this->writeText(retStr.c_str(), retStr.length());
			}
		};
	}
}
#endif // !_doyou_io_INetClientS_HPP_
