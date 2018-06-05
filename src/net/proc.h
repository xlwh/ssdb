/*
Copyright (c) 2012-2014 The SSDB Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
*/
#ifndef NET_PROC_H_
#define NET_PROC_H_

#include <vector>
#include "resp.h"
#include "../util/bytes.h"

class Link;
class NetworkServer;

#define PROC_OK			0
#define PROC_ERROR		-1
#define PROC_THREAD     1
#define PROC_BACKEND	100

#define DEF_PROC(f) int proc_##f(NetworkServer *net, Link *link, const Request &req, Response *resp)

typedef std::vector<Bytes> Request;
typedef int (*proc_t)(NetworkServer *net, Link *link, const Request &req, Response *resp);

// 命令列表的结构
struct Command{
	static const int FLAG_READ		= (1 << 0);     // 读操作命令
	static const int FLAG_WRITE		= (1 << 1);     // 写操作命令
	static const int FLAG_BACKEND	= (1 << 2);
	static const int FLAG_THREAD	= (1 << 3);

	std::string name;     	// 命令名
	int flags;              // 操作标记
	proc_t proc;          	// 处理函数
	uint64_t calls;        	// 命令的调用次数
	double time_wait;      	// 命令处理的超时时间
	double time_proc;
	
	Command(){
		flags = 0;
		proc = NULL;
		calls = 0;
		time_wait = 0;
		time_proc = 0;
	}
};

struct ProcJob{
	int result;
	NetworkServer *serv;
	Link *link;        // 具体的连接
	Command *cmd;      // 具体的操作命令
	double stime;     // in seconds
	double time_wait; // in ms
	double time_proc; // in ms
	
	// 请求是不可修改的
	const Request *req;   // 请求
	Response resp;        // 响应数据
	
	ProcJob(){
		result = 0;
		serv = NULL;
		link = NULL;
		cmd = NULL;
		stime = 0;
		time_wait = 0;
		time_proc = 0;
	}
	~ProcJob(){
	}
};


struct BytesEqual{
	bool operator()(const Bytes &s1, const Bytes &s2) const {
		return (bool)(s1.compare(s2) == 0);
	}
};
struct BytesHash{
	size_t operator()(const Bytes &s1) const {
		unsigned long __h = 0;
		const char *p = s1.data();
		for (int i=0 ; i<s1.size(); i++)
			__h = 5*__h + p[i];
		return size_t(__h);
	}
};


#define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#if GCC_VERSION >= 403
	#include <tr1/unordered_map>
	typedef std::tr1::unordered_map<Bytes, Command *, BytesHash, BytesEqual> proc_map_t;
#else
	#ifdef NEW_MAC
		#include <unordered_map>
		typedef std::unordered_map<Bytes, Command *, BytesHash, BytesEqual> proc_map_t;
	#else
		#include <ext/hash_map>
		typedef __gnu_cxx::hash_map<Bytes, Command *, BytesHash, BytesEqual> proc_map_t;
	#endif
#endif


class ProcMap
{
private:
	proc_map_t proc_map;

public:
	ProcMap();
	~ProcMap();
	// 设置请求
	void set_proc(const std::string &cmd, const char *sflags, proc_t proc);
	void set_proc(const std::string &cmd, proc_t proc);
	Command* get_proc(const Bytes &str);
	
	proc_map_t::iterator begin(){
		return proc_map.begin();
	}
	proc_map_t::iterator end(){
		return proc_map.end();
	}
};



#include "../util/strings.h"

template<class T>
static std::string serialize_req(T &req){
	std::string ret;
	ret.reserve(1024);
	char buf[50];
	for(int i=0; i<req.size(); i++){
		if(i >= 5 && i < req.size() - 1){
			sprintf(buf, "[%d more...]", (int)req.size() - i);
			ret.append(buf);
			break;
		}
		if(req[i].size() < 50){
			if(req[i].size() == 0){
				ret.append("\"\"");
			}else{
				str_escape(req[i].data(), req[i].size(), &ret);
			}
		}else{
			sprintf(buf, "[%d]", (int)req[i].size());
			ret.append(buf);
		}
		if(i < req.size() - 1){
			ret.append(" ");
		}
	}
	return ret;
}

#endif
