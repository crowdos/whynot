#include "plugin.h"
#include <map>
#include <map>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

std::map<std::string, int> devices = {
  {"/dev/watchdog", 20},
  {"/dev/twl4030_wdt", 30},
};

void kick_wd(boost::asio::deadline_timer *t, int& fd, int time)
{
  if (write(fd, "*", 1) != 1) {
    std::cerr << "Failed to kick watchdog: " << std::strerror(errno) << std::endl;
  }

  t->expires_at(t->expires_at() + boost::posix_time::seconds(time));
  t->async_wait(boost::bind(kick_wd, t, fd, time));
}

class WatchdogPlugin : public Plugin {
public:
  bool start(boost::asio::io_service& service) {
    for (auto& wd : devices) {
      int fd = open(wd.first.c_str(), O_RDWR);
      if (fd == -1) {
	std::cerr << "Failed to open " << wd.first << ": " << std::strerror(errno) << std::endl;
	continue;
      }

      int tmp = wd.second;
      ioctl(fd, WDIOC_SETTIMEOUT, &tmp);

      boost::asio::deadline_timer
	*t(new boost::asio::deadline_timer(service, boost::posix_time::seconds(wd.second)));

      kick_wd(t, fd, wd.second);

      m_timers.insert(std::make_pair(fd, t));
    }
  }

  void stop() {
    for (auto& wd : m_timers) {
      wd.second->cancel();
      delete wd.second;
      close(wd.first);
    }

    m_timers.clear();
  }

private:
  std::map<int, boost::asio::deadline_timer *> m_timers;
};

REGISTER_PLUGIN(WatchdogPlugin)
