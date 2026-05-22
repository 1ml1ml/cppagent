module;

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <cstring>
#include <variant>
#include <stdexcept>

#include <windows.h>

#include "nlohmann/json.hpp"

module stdio_transport;

import jsonrpc;
import transport;

class stdio_transport::impl
{
public:
	static std::string quote_arg(const std::string& arg);
	static std::string build_env_block(std::map<std::string, std::string> envs);
	static std::string build_command_line(const std::string& cmd, const std::vector<std::string>& args);

	static void create_pipe(HANDLE& read, HANDLE& write);
	static void set_inherit(HANDLE h, bool inherit);

	static jsonrpc_message parse_line(const std::string& line);

public:
	void start_receive_thread();

public:
	std::string cmd_line{};
	std::string env_block{};

	HANDLE stdin_write{};
	HANDLE stdout_read{};
	PROCESS_INFORMATION proc_info{};

	bool connected{ false };
	std::thread read_thread{};
	transport::receive_callback callback{};
};

std::string stdio_transport::impl::quote_arg(const std::string& arg)
{
	if (arg.find(' ') == std::string::npos && arg.find('"') == std::string::npos)
	{
		return arg;
	}
	return "\"" + arg + "\"";
}

std::string stdio_transport::impl::build_env_block(std::map<std::string, std::string> envs)
{
	if (envs.empty())
	{
		return {};
	}

	if (auto parent_env{ GetEnvironmentStringsA() }; parent_env != nullptr)
	{
		for (auto index{ parent_env }; *index != '\0'; )
		{
			auto len{ std::strlen(index) };
			std::string entry{ index, len };

			if (auto pos{ entry.find('=') }; pos != std::string::npos && pos > 0)
			{
				if (auto name{ entry.substr(0, pos) }; !envs.contains(name))
				{
					envs[name] = entry.substr(pos + 1);
				}
			}
			index += len + 1;
		}
		FreeEnvironmentStringsA(parent_env);
	}

	std::string block{};
	for (const auto& [key, value] : envs)
	{
		block += key + "=" + value + '\0';
	}
	block += '\0';

	return block;
}

std::string stdio_transport::impl::build_command_line(const std::string& cmd, const std::vector<std::string>& args)
{
	auto result{ "cmd.exe /c " + quote_arg(cmd) };
	for (const auto& arg : args)
	{
		result += " " + quote_arg(arg);
	}
	return result;
}

void stdio_transport::impl::create_pipe(HANDLE& read, HANDLE& write)
{
	SECURITY_ATTRIBUTES sa{};
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = nullptr;

	if (!CreatePipe(&read, &write, &sa, 0))
	{
		throw std::runtime_error{ "Failed to create pipe" };
	}
}

jsonrpc_message stdio_transport::impl::parse_line(const std::string& line)
{
	auto j{ nlohmann::json::parse(line) };

	if (j.contains("id") && (j.contains("result") || j.contains("error")))
	{
		return jsonrpc_response::from_json(j);
	}

	if (j.contains("id") && j.contains("method"))
	{
		return jsonrpc_request::from_json(j);
	}

	if (j.contains("method"))
	{
		return jsonrpc_notification::from_json(j);
	}

	throw std::runtime_error{ "Unknown JSON-RPC message type" };
}

void stdio_transport::impl::set_inherit(HANDLE h, bool inherit)
{
	if (!SetHandleInformation(h, HANDLE_FLAG_INHERIT, inherit ? HANDLE_FLAG_INHERIT : 0))
	{
		throw std::runtime_error{ "Failed to set handle inheritance" };
	}
}

void stdio_transport::impl::start_receive_thread()
{
	if (read_thread.joinable())
	{
		throw std::runtime_error{ "Read thread already running" };
	}

	read_thread = std::thread{ [this]
	{
		std::string cache{};

		while (connected)
		{
			DWORD read{};
			char buf[4096]{};

			if (ReadFile(stdout_read, buf, sizeof(buf), &read, nullptr) || read == 0)
			{
				cache.append(buf, read);

				while (connected)
				{
					auto pos{ cache.find('\n') };
					if (pos == std::string::npos)
					{
						break;
					}

					auto line{ cache.substr(0, pos) };
					cache.erase(0, pos + 1);

					if (callback && line.size())
					{
						callback(parse_line(line));
					}
				}
			}
		}
	} };
}

stdio_transport::stdio_transport(const std::string& command, const std::vector<std::string>& args, const std::map<std::string, std::string>& envs)
  : transport(),
    impl{ std::make_unique<class impl>() }
{
	impl->env_block = impl::build_env_block(envs);
	impl->cmd_line = impl::build_command_line(command, args);
}

stdio_transport::~stdio_transport()
{
	close();
}

void stdio_transport::set_receive_callback(const receive_callback& callback)
{
	impl->callback = callback;
}

void stdio_transport::open()
{
	if (impl->connected)
	{
		throw std::runtime_error{ "Transport already open" };
	}

	HANDLE stdin_read{};
	HANDLE stdout_write{};

	impl::create_pipe(stdin_read, impl->stdin_write);
	impl::create_pipe(impl->stdout_read, stdout_write);

	impl::set_inherit(stdin_read, TRUE);
	impl::set_inherit(stdout_write, TRUE);
	impl::set_inherit(impl->stdin_write, FALSE);
	impl::set_inherit(impl->stdout_read, FALSE);

	STARTUPINFOA si{};
	si.cb = sizeof(STARTUPINFOA);
	si.hStdInput = stdin_read;
	si.hStdOutput = stdout_write;
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si.dwFlags = STARTF_USESTDHANDLES;

	auto success{ CreateProcessA(nullptr, impl->cmd_line.data(), nullptr, nullptr, TRUE, 0, impl->env_block.empty() ? nullptr : impl->env_block.data(), nullptr, &si, &impl->proc_info) };

	CloseHandle(stdin_read);
	CloseHandle(stdout_write);

	if (!success)
	{
		CloseHandle(impl->stdin_write);
		CloseHandle(impl->stdout_read);

		impl->stdin_write = nullptr;
		impl->stdout_read = nullptr;

		throw std::runtime_error{ "Failed to start process: " + impl->cmd_line };
	}

	impl->connected = true;
	impl->start_receive_thread();
}

void stdio_transport::send(const jsonrpc_message& msg)
{
	if (!impl->connected)
	{
		throw std::runtime_error{ "Transport not connected" };
	}

	auto data{ std::visit([](const auto& jsonrpc) -> nlohmann::json { return jsonrpc.to_json(); }, msg).dump() + "\n" };
	if (DWORD written{}; !WriteFile(impl->stdin_write, data.data(), static_cast<DWORD>(data.size()), &written, nullptr))
	{
		throw std::runtime_error{ "Failed to write to process stdin: " + data };
	}
}

bool stdio_transport::is_connected()
{
	return impl->connected;
}

void stdio_transport::close()
{
	if (!impl->connected)
	{
		return;
	}

	impl->connected = false;

	if (impl->stdin_write != nullptr)
	{
		CloseHandle(impl->stdin_write);
		impl->stdin_write = nullptr;
	}

	if (impl->read_thread.joinable())
	{
		CancelSynchronousIo(impl->read_thread.native_handle());
		impl->read_thread.join();
	}

	if (impl->stdout_read != nullptr)
	{
		CloseHandle(impl->stdout_read);
		impl->stdout_read = nullptr;
	}

	if (impl->proc_info.hProcess != nullptr)
	{
		TerminateProcess(impl->proc_info.hProcess, 1);
		WaitForSingleObject(impl->proc_info.hProcess, INFINITE);

		CloseHandle(impl->proc_info.hProcess);
		impl->proc_info.hProcess = nullptr;
	}

	if (impl->proc_info.hThread)
	{
		CloseHandle(impl->proc_info.hThread);
		impl->proc_info.hThread = nullptr;
	}
}
