#include "websocket_signaller.h"
#include <cassert>

namespace grt {
	void signaller::connect(
		std::string host, std::string port, std::string text, std::shared_ptr<signaller_callback> clb) {
		assert(false);//this function should be overidden if any client calls this funtion
	}

	std::unique_ptr<signaller>
		get_signaller_handle() {
		return std::make_unique< websocket_signaller>();
	}
}
