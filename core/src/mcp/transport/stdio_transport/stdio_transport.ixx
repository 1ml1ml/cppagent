module;

#include <map>
#include <memory>
#include <string>
#include <vector>

export module stdio_transport;

import transport;

export class stdio_transport : public transport
{
public:
  stdio_transport(const std::string& command, const std::vector<std::string>& args, const std::map<std::string, std::string>& envs = {});
  ~stdio_transport();

public:
  void set_receive_callback(const receive_callback& callback) override;

  void open() override;
  void close() override;

  void send(const jsonrpc_message& msg) override;

  bool is_connected() override;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};