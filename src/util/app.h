#ifndef UTIL_APP_H
#define UTIL_APP_H

#include <string>

class Config;

// 定义了一个公共的父类
class Application{
public:
	Application(){};
	virtual ~Application(){};

	// 函数的入口
	int main(int argc, char **argv);
	
	virtual void usage(int argc, char **argv);
	virtual void welcome() = 0;
	virtual void run() = 0;

protected:
	struct AppArgs{
		bool is_daemon;   		 // 是否在后台执行
		std::string pidfile;     // pid保存在哪
		std::string conf_file;   // 配置文件在哪
		std::string work_dir;    // 工作目录是什么
		std::string start_opt;   // 启动命令

		AppArgs(){
			is_daemon = false;
			start_opt = "start";
		}
	};

	Config *conf;
	AppArgs app_args;
	
private:
	void parse_args(int argc, char **argv);
	void init();

	int read_pid();
	void write_pid();
	void check_pidfile();
	void remove_pidfile();
	void kill_process();
};

#endif
