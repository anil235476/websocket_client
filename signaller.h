#ifndef _SIGNALLER_H__
#define _SIGNALLER_H__
#include <string>
#include <memory>

namespace grt {

	class signaller_callback {
	public:
		virtual ~signaller_callback() {}
		virtual void on_message(std::string msg){}
		virtual void on_connect(){}
		virtual void on_error(std::string error){}
		virtual void on_close(){}
	};

	class signaller {
	public:
		virtual ~signaller(){}
		virtual void connect(std::string host, std::string port, std::shared_ptr<signaller_callback> clb) = 0;
		virtual void connect(std::string host, std::string port, std::string text, std::shared_ptr<signaller_callback> clb);
		virtual void set_callback(signaller_callback* clb) {}
		virtual void disconnect(){}
		virtual void send(std::string msg) = 0;
	};

	std::unique_ptr<signaller>
		get_signaller_handle();

}//namespace grt

#endif//_SIGNALLER_H__