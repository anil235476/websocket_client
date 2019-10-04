#ifndef _WEBSOCKET_SIGNALLER_H_
#define _WEBSOCKET_SIGNALLER_H_
#include "signaller.h"
#include <memory>
#include <thread>

namespace detail {
	class session;
	class session_unsecure;
}

namespace grt {

	class websocket_signaller : public signaller {
	public:
		~websocket_signaller() override;
		void connect(std::string host, std::string port, std::shared_ptr<signaller_callback> clb) override;
		void connect(std::string host, std::string port, std::string text, std::shared_ptr<signaller_callback> clb) override;
		void set_callback(std::shared_ptr<signaller_callback> clb) override;
		void disconnect() override;
		void send(std::string msg) override;

	private:
#ifdef SECURE_SIGNALING
		std::shared_ptr<detail::session> session_;
#else
		std::shared_ptr<detail::session_unsecure> session_;
#endif
		std::thread t_;
	};

	class websocket_signaller_unsecure : public signaller {
	public:
		~websocket_signaller_unsecure() override;
		void connect(std::string host, std::string port, std::shared_ptr<signaller_callback> clb) override;
		void connect(std::string host, std::string port, std::string text, std::shared_ptr<signaller_callback> clb) override;
		void set_callback(std::shared_ptr<signaller_callback> clb) override;
		void disconnect() override;
		void send(std::string msg) override;

	private:
		std::shared_ptr<detail::session_unsecure> session_;
		std::thread t_;
	};

}//namespace grt


#endif//_WEBSOCKET_SIGNALLER_H_