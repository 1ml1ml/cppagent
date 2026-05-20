module;

#include <memory>
#include <string>
#include <vector>
#include <functional>

export module transport;

import jsonrpc;

class transport;
export using transport_shared_ptr = std::shared_ptr<transport>;
export using transport_unique_ptr = std::unique_ptr<transport>;

export class transport : public std::enable_shared_from_this<transport>
{
public:
  virtual ~transport() = default;

public:
  using receive_callback = std::function<void(const jsonrpc_message&)>;

  virtual void set_receive_callback(const receive_callback& callback) = 0;

  virtual void open() = 0;
  virtual void close() = 0;
  
  virtual void send(const jsonrpc_message& msg) = 0;

  virtual bool is_connected() = 0;
};
