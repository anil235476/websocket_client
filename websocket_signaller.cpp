#include "websocket_signaller.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace detail {
	//------------------------------------------------------------------------------

	// Report a failure
	void
		fail(boost::system::error_code ec, char const* what)
	{
		std::cerr << what << ": " << ec.message() << "\n";
	}

	// Sends a WebSocket message and prints the response
	class session{
		tcp::resolver resolver_;
		websocket::stream<tcp::socket> ws_;
	    boost::beast::multi_buffer buffer_;
		std::string host_;
		std::string text_{ "/" };
		std::shared_ptr<grt::signaller_callback> callbck_{ nullptr };
	public:
		// Resolver and socket require an io_context
		explicit
			session(boost::asio::io_context& ioc, std::shared_ptr<grt::signaller_callback> callbk)
			: resolver_(ioc)
			, ws_(ioc)
			, callbck_{ callbk }
		{
			assert(callbck_);
		}

		// Start the asynchronous operation
		void
			run(
				std::string host,
				std::string port, std::string text) {
			// Save these for later
			host_ = host;
			if(!text.empty())
		    text_ = text;

			 // Look up the domain name
			resolver_.async_resolve(
				host,
				port,
				std::bind(
					&session::on_resolve,
					this,
					std::placeholders::_1,
					std::placeholders::_2));
		}

		void
			on_resolve(
				boost::system::error_code ec,
				tcp::resolver::results_type results)
		{
			if (ec)
				return fail(ec, "resolve");

			// Make the connection on the IP address we get from a lookup
			boost::asio::async_connect(
				ws_.next_layer(),
				results.begin(),
				results.end(),
				std::bind(
					&session::on_connect,
					this,
					std::placeholders::_1)
			);
		}

		void
			on_connect(boost::system::error_code ec)
		{
			if (ec)
				return fail(ec, "connect");

			// Perform the websocket handshake
			ws_.async_handshake(host_, text_,
				std::bind(
					&session::on_handshake,
					this,
					std::placeholders::_1));
		}

		void send_message(std::string msg_) {
			const auto r = ws_.write(boost::asio::buffer(msg_));
			assert(r == msg_.size());
		}

		void
			on_handshake(boost::system::error_code ec)
		{
			if (ec)
				return fail(ec, "handshake");
			
			start_reading();
			callbck_->on_connect();
			/*std::thread{
				[this](){callbck_->on_connect(); }
			}.detach();*/
			
		}

		void
			start_reading() {
			// Read a message into our buffer
			ws_.async_read(
				buffer_,
				std::bind(
					&session::on_read,
					this,
					std::placeholders::_1,
					std::placeholders::_2)
			);
		}

		void
			on_read(
				boost::system::error_code ec,
				std::size_t bytes_transferred) {

			boost::ignore_unused(bytes_transferred);

			if (ec)
				return fail(ec, "read");
			callbck_->on_message(boost::beast::buffers_to_string(buffer_.data()));
			buffer_.consume(buffer_.size());
			start_reading();
		}

		void close() {
			// Close the WebSocket connection
			ws_.async_close(websocket::close_code::normal,
				std::bind(
					&session::on_close,
					this,
					std::placeholders::_1));

		}

		void
			on_close(boost::system::error_code ec)
		{
			if (ec)
				return fail(ec, "close");
			callbck_->on_close();

			// std::cout << boost::beast::buffers(buffer_.data()) << std::endl;
		}

		void set_callback(std::shared_ptr<grt::signaller_callback> callbk) {
			assert(callbk);
			callbck_ = callbk;
		}
	};
}//namespace detail

namespace grt {

	void websocket_signaller::connect(std::string host,
		std::string port, std::shared_ptr<signaller_callback> clb) {
		connect(host, port, std::string{"/"}, clb);
		/*t_ = std::thread{ [this, host, port, clb]() {
			boost::asio::io_context ioc;
			session_ = std::make_shared<detail::session>(
				ioc, clb);
			session_->run(host, port);
			ioc.run();
			}
		};*/
	}

	void websocket_signaller::connect(std::string host, std::string port,
		std::string text, std::shared_ptr<signaller_callback> clb) {
		t_ = std::thread{ [this, host, port, text, clb]() {
			boost::asio::io_context ioc;
			session_ = std::make_shared<detail::session>(
				ioc, clb);
			session_->run(host, port, text);
			ioc.run();
			}
		};
	}

	void websocket_signaller::set_callback(std::shared_ptr<signaller_callback> clb) {
		session_->set_callback(clb);
	}

	void websocket_signaller::disconnect() {
		session_->close();
	}

	websocket_signaller::~websocket_signaller() {
		t_.join();
	}

	void websocket_signaller::send(std::string msg) {
		session_->send_message(msg);
	}

}//namespace grt