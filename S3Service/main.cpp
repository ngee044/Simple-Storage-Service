#include "Configurations.h"
#include "S3Service.h"

#include "fmt/format.h"
#include "fmt/xchar.h"

#include <iostream>
#include <string>
#include <signal.h>

void register_signal(void);
void deregister_signal(void);
void signal_callback(int32_t signum);

using namespace S3Service;
using namespace Utilities;

std::shared_ptr<S3ServiceMain> s3_service_main_;
std::shared_ptr<Configurations> configurations_;


auto main(int argc, char* argv[]) -> int
{
	try
	{
		configurations_ = std::make_shared<Configurations>(ArgumentParser(argc, argv));
	}
	catch (const std::bad_alloc& e)
	{
		std::cerr << "Memory allocation failed to configuration: " << e.what() << std::endl;

		return 0;
	}

	Logger::handle().file_mode(configurations_->write_file());
	Logger::handle().console_mode(configurations_->write_console());
	Logger::handle().write_interval(configurations_->write_interval());
	Logger::handle().log_root(configurations_->log_root_path());

	Logger::handle().start(configurations_->client_title());

	register_signal();

	s3_service_main_ = std::make_shared<S3ServiceMain>(configurations_);
	auto [result, message] = s3_service_main_->start();
	if (result)
	{
		s3_service_main_->wait_stop();
	}

	s3_service_main_.reset();

	Logger::handle().stop();
	Logger::destroy();

	return 0;
}

void register_signal(void)
{
	signal(SIGINT, signal_callback);
	signal(SIGILL, signal_callback);
	signal(SIGABRT, signal_callback);
	signal(SIGFPE, signal_callback);
	signal(SIGSEGV, signal_callback);
	signal(SIGTERM, signal_callback);
}

void deregister_signal(void)
{
	signal(SIGINT, nullptr);
	signal(SIGILL, nullptr);
	signal(SIGABRT, nullptr);
	signal(SIGFPE, nullptr);
	signal(SIGSEGV, nullptr);
	signal(SIGTERM, nullptr);
}

void signal_callback(int32_t signum)
{
	deregister_signal();
	// TODO
	// Handle signal
	// Log the signal
	// main loop should be stopped
}