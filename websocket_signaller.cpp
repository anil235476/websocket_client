#include "websocket_signaller.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;               // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace detail {
	//------------------------------------------------------------------------------

	// Report a failure
	void
		fail(boost::system::error_code ec, char const* what, grt::signaller_callback* callbck)
	{
		const auto m = ec.message();
		std::cerr << what << ": " << m << "\n";
		assert(callbck);
		
		callbck->on_error(m + what);
	}

	// Sends a WebSocket message and prints the response
	class session{
		tcp::resolver resolver_;
		websocket::stream<ssl::stream<tcp::socket>> ws_;
	    boost::beast::multi_buffer buffer_;
		std::string host_;
		std::string text_{ "/" };
		grt::signaller_callback* callbck_{ nullptr };
	public:
		// Resolver and socket require an io_context
		explicit
			session(boost::asio::io_context& ioc, ssl::context& ctx,grt::signaller_callback* callbk)
			: resolver_(ioc)
			, ws_(ioc, ctx)
			, callbck_{ callbk }
		{
			assert(callbck_);
		}
		
		~session() {
#ifdef _DEBUG
			//assert(false);//just to know if it is called.
			std::cout << "~session destructor called\n";
#endif//_DEBUG
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
			if (ec) {
				return fail(ec, "resolve", callbck_);
			}
				

			// Make the connection on the IP address we get from a lookup
			boost::asio::async_connect(
				ws_.next_layer().next_layer(),
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
				return fail(ec, "connect", callbck_);
			ws_.next_layer().async_handshake(ssl::stream_base::client,
				std::bind(
					&session::on_ssl_handshake,
					this,
					std::placeholders::_1));
		}

		void
			on_ssl_handshake(boost::system::error_code ec)
		{
			if (ec) {
				return fail(ec, "ssl_handshake", callbck_);
			}
				

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
				return fail(ec, "handshake", callbck_);
			
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

			if (ec) {
				//assert(false);
				return fail(ec, "read", callbck_);
			}
				
			callbck_->on_message(boost::beast::buffers_to_string(buffer_.data()));
			buffer_.consume(buffer_.size());
			start_reading();
		}

		void close() {
			//ws_.async_close()
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
				return fail(ec, "close", callbck_);
			callbck_->on_close();

			// std::cout << boost::beast::buffers(buffer_.data()) << std::endl;
		}

		void set_callback(grt::signaller_callback* callbk) {
			assert(callbk);
			callbck_ = callbk;
		}
	};


	// Sends a WebSocket message and prints the response
	class session_unsecure {
		tcp::resolver resolver_;
		websocket::stream<tcp::socket> ws_;
		boost::beast::multi_buffer buffer_;
		std::string host_;
		std::string text_{ "/" };
		grt::signaller_callback* callbck_{ nullptr };
	public:
		// Resolver and socket require an io_context
		explicit
			session_unsecure(boost::asio::io_context& ioc, ssl::context& ctx, grt::signaller_callback* callbk)
			: resolver_(ioc)
			, ws_(ioc)
			, callbck_{ callbk }
		{
			assert(callbck_);
		}

		~session_unsecure() {
#ifdef _DEBUG
			std::cout << "~session_unsecure destructor called, host ="<<host_<<'\n';
#endif//_DEBUG
		}

		// Start the asynchronous operation
		void
			run(
				std::string host,
				std::string port, std::string text) {
			// Save these for later
			host_ = host;
			if (!text.empty())
				text_ = text;

			// Look up the domain name
			resolver_.async_resolve(
				host,
				port,
				std::bind(
					&session_unsecure::on_resolve,
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
				return fail(ec, "resolve", callbck_);

			// Make the connection on the IP address we get from a lookup
			boost::asio::async_connect(

				ws_.next_layer(),
				results.begin(),
				results.end(),
				std::bind(
					&session_unsecure::on_connect,
					this,
					std::placeholders::_1)
			);
		}

		void
			on_connect(boost::system::error_code ec)
		{
			if (ec)
				return fail(ec, "connect", callbck_);
			// Perform the websocket handshake
			ws_.async_handshake(host_, text_,
				std::bind(
					&session_unsecure::on_handshake,
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
				return fail(ec, "handshake", callbck_);

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
					&session_unsecure::on_read,
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
				return fail(ec, "read", callbck_);
			callbck_->on_message(boost::beast::buffers_to_string(buffer_.data()));
			buffer_.consume(buffer_.size());
			start_reading();
		}

		void close() {
			//ws_.async_close()
			// Close the WebSocket connection
			if(ws_.is_open())
				ws_.async_close(websocket::close_code::normal,
					std::bind(
						&session_unsecure::on_close,
						this,
						std::placeholders::_1));

		}

		void
			on_close(boost::system::error_code ec)
		{
			if (ec)
				return fail(ec, "close", callbck_);
			callbck_->on_close();

			// std::cout << boost::beast::buffers(buffer_.data()) << std::endl;
		}

		void set_callback(grt::signaller_callback* callbk) {
			assert(callbk);
			callbck_ = callbk;
		}
	};
}//namespace detail

namespace grt {
	std::string extract_charcter_from_end(std::string v) {
		const auto count = v.find('\n');
		return v.substr(0, count);
	}
	void websocket_signaller::connect(std::string host,
		std::string port, std::shared_ptr<signaller_callback> clb) {
		connect(extract_charcter_from_end(host), extract_charcter_from_end(port), std::string{"/"}, clb);
	}

	void websocket_signaller::connect(std::string host, std::string port,
		std::string text, std::shared_ptr<signaller_callback> clb) {
		clb_ = clb;
		t_ = std::thread{ [this, host, port, text, clb]() {
			boost::asio::io_context ioc;
			// The SSL context is required, and holds certificates
			ssl::context ctx{ ssl::context::sslv23_client };

			session_ = std::make_shared<session_imp>(
				ioc, ctx, clb.get());
			session_->run(host, port, text);
			ioc.run();
			std::cout << "\n coming out of session run \n";
			session_.reset();
			}
		};
	}

	void websocket_signaller::set_callback(signaller_callback* clb ) {
		clb_.reset();
		if(session_)
			session_->set_callback(clb);
	}

	void websocket_signaller::disconnect() {
		if (session_) {
			session_->close();
			t_.join();
		}
			
	}

	websocket_signaller::~websocket_signaller() {
		std::cout << "\n~websocket_signaller() called\n";
		if(t_.joinable())
				t_.join();
	}

	void websocket_signaller::send(std::string msg) {
		try {
			if(session_)
				session_->send_message(msg);
		}
		catch (boost::exception const& /*ex*/) {
			//boost::error_info(ex);
			//todo: handle this.
		}
	}


	void websocket_signaller_unsecure::connect(std::string host,
		std::string port, std::shared_ptr<signaller_callback> clb) {
		connect(extract_charcter_from_end(host), extract_charcter_from_end(port), std::string{ "/" }, clb);
	}

	void websocket_signaller_unsecure::connect(std::string host, std::string port,
		std::string text, std::shared_ptr<signaller_callback> clb) {
		clb_ = clb;
		t_ = std::thread{ [this, host, port, text, clb]() {
			boost::asio::io_context ioc;
			// The SSL context is required, and holds certificates
			ssl::context ctx{ ssl::context::sslv23_client };

			session_ = std::make_shared<detail::session_unsecure>(
				ioc, ctx, clb.get());
			session_->run(host, port, text);
			ioc.run();
			std::cout << "\n coming out of session run websocket_signaller_unsecure \n";
			session_.reset();
			}
		};
	}

	void websocket_signaller_unsecure::set_callback(signaller_callback* clb) {
		clb_.reset();
		if(session_)
			session_->set_callback(clb);
	}

	void websocket_signaller_unsecure::disconnect() {
		if(session_)
			session_->close();
	}

	websocket_signaller_unsecure::~websocket_signaller_unsecure() {
		t_.join();
	}

	void websocket_signaller_unsecure::send(std::string msg) {
		if(session_)
			session_->send_message(msg);
	}


}//namespace grt