#include <vector>

template <typename... Args>
class Signal
{
  public:
	using Callback = std::function<void(Args...)>;

	void subscribe(Callback cb)
	{
		callbacks.push_back(std::move(cb));
	}

	void invoke(Args... args)
	{
		for (const auto &cb : callbacks)
		{
			cb(args...);
		}
	}

  private:
	std::vector<Callback> callbacks;
};