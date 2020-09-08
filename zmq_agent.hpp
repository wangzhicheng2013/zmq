#ifndef SRC_ZMQ_AGENT_HPP_
#define SRC_ZMQ_AGENT_HPP_
#include <string.h>
#include <string>
#include <map>
#include "zmq.h"
namespace zmq_self_agent {
using namespace std;
enum {
	BIND,
	CONNECT
};
enum {
	NO_ERROR,
	CONTEXT_ERROR,
	SOCKET_ERROR,
	SOCKET_TYPE_ERROR,
	BIND_ERROR,
	CONNECT_ERROR,
	SOCKET_ATTRIBUTE_ERROR
};
class zmq_config {
public:
	zmq_config() {
		sock_type = 0;
		addr = nullptr;
		send_hwm = 1000;
		recv_hwm = 1000;
		send_timeout = 5;
	}
public:
	unsigned char sock_type;
	const char *addr;
	int send_hwm;
	int recv_hwm;
	int send_timeout;
};
class zmq_agent {
public:
	zmq_agent() {
		context_ = nullptr;
		socket_ = nullptr;
		init_map();
	}
	virtual ~zmq_agent() {
	    if (nullptr != socket_) {
	        zmq_close(socket_);
	        socket_= nullptr;
	    }
	    if (nullptr != context_) {
	        zmq_ctx_destroy(context_);
	        context_ = nullptr;
	    }
	}
public:
	unsigned char init(const zmq_config &config) {
		unsigned char ret = init_socket(config);
		if (ret) {
			return ret;
		}
		ret = init_link(config);
		if (ret) {
			return ret;
		}
		ret = init_sock_attribute(config);
		return ret;
	}
	int send(const char *info, int len) {
	    zmq_msg_t msg = {0};
	    int rc = zmq_msg_init_size(&msg, len);
	    if (rc) {
	    	return 0;
	    }
	    memcpy(zmq_msg_data(&msg), info, len);
	    rc = zmq_msg_send(&msg, socket_, 0);
		zmq_msg_close(&msg);
	    return rc;
	}
	int send(const string &info) {
	    zmq_msg_t msg = {0};
		int len = info.size();
	    int rc = zmq_msg_init_size(&msg, len);
	    if (rc) {
	    	return 0;
	    }
	    memcpy(zmq_msg_data(&msg), info.c_str(), len);
	    rc = zmq_msg_send(&msg, socket_, 0);
		zmq_msg_close(&msg);
	    return rc;
	}
	bool recv(string &recv_str) {
		zmq_msg_t msg = {0};
		int rc = zmq_msg_init(&msg);
		if (rc) {
			return false;
		}
		int len = zmq_msg_recv(&msg, socket_, 0);
		bool succ = false;
		if (len > 0) {
			recv_str.assign((char *)zmq_msg_data(&msg), len);
			succ = true;
		}
		zmq_msg_close(&msg);
		return succ;
	}
	static void get_version(string &str) {
		int major, minor, patch;
		zmq_version(&major, &minor, &patch);
		char buf[64] = "";
		snprintf(buf, sizeof(buf), "%d.%d.%d", major, minor, patch);
		str = buf;
	}
private:
	inline void init_map() {
		sock_type_map_[ZMQ_PULL] = BIND;
		sock_type_map_[ZMQ_PUB] = BIND;
 
		sock_type_map_[ZMQ_PUSH] = CONNECT;
		sock_type_map_[ZMQ_SUB] = CONNECT;
	}
	inline unsigned char init_socket(const zmq_config &config) {
		if (nullptr == (context_ = zmq_ctx_new())) {
			return CONTEXT_ERROR;
		}
	    if (nullptr == (socket_ = zmq_socket(context_, config.sock_type))) {
	    	return SOCKET_ERROR;
	    }
	    return NO_ERROR;
	}
	unsigned char init_link(const zmq_config &config) {
		auto it = sock_type_map_.find(config.sock_type);
		if (end(sock_type_map_) == it) {
			return SOCKET_TYPE_ERROR;
		}
		unsigned char ret = 0;
		if (BIND == it->second) {
			ret = zmq_bind(socket_, config.addr);
			if (ret) {
				return BIND_ERROR;
			}
			return NO_ERROR;
		}
		ret = zmq_connect(socket_, config.addr);
		if (ret) {
			return CONNECT_ERROR;
		}
		return NO_ERROR;
	}
	inline unsigned char init_sock_attribute(const zmq_config &config) {
		unsigned char ret = zmq_setsockopt(socket_, ZMQ_SNDHWM, &config.send_hwm, sizeof(config.send_hwm));
	    if (ret) {
	    	return SOCKET_ATTRIBUTE_ERROR;
	    }
	    ret = zmq_setsockopt(socket_, ZMQ_RCVHWM, &config.recv_hwm, sizeof(config.recv_hwm));
	    if (ret) {
	    	return SOCKET_ATTRIBUTE_ERROR;
	    }
	    ret = zmq_setsockopt(socket_, ZMQ_SNDTIMEO, &config.send_timeout, sizeof(config.send_timeout));
	    if (ret) {
	    	return SOCKET_ATTRIBUTE_ERROR;
	   	}
	    return NO_ERROR;
	}
private:
    void *context_;
    void *socket_;
private:
    map<unsigned char, unsigned char>sock_type_map_;
};
}
 
#endif /* SRC_ZMQ_AGENT_HPP_ */