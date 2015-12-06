#include "plugin.h"
#include <list>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

#define TIMEOUT 10

std::map<std::string, int> devices = {
  {"/dev/watchdog", 20},
  {"/dev/twl4030_wdt", 30},
};

void kick_wd(boost::asio::deadline_timer *t, int& fd, int time)
{
  t->expires_at(t->expires_at() + boost::posix_time::seconds(time));
  t->async_wait(boost::bind(kick_wd, t, fd, time));
}

class WatchdogPlugin : public Plugin {
public:
  WatchdogPlugin() :
    m_timer(nullptr) { }

  bool start(boost::asio::io_service& service) {
    for (auto& wd : devices) {
      int fd = open(wd.first.c_str(), O_RDWR);
      if (fd == -1) {
	std::cerr << "Failed to open " << wd.first << ": " << std::strerror(errno) << std::endl;
	continue;
      }

      // We don't care if it fails
      int tmp = wd.second;
      ioctl(fd, WDIOC_SETTIMEOUT, &tmp);

      m_fds.push_back(fd);
    }

    if (m_fds.size() > 0) {
      m_timer = new boost::asio::deadline_timer(service, boost::posix_time::seconds(TIMEOUT));
      tick(true);
    }

    return m_fds.size() > 0;
  }

  void stop() {
    for (int fd : m_fds) {
      close(fd);
    }

    m_fds.clear();
    if (m_timer) {
      m_timer->cancel();
      delete m_timer;
      m_timer = nullptr;
    }
  }

private:
  void tick(bool initial_tick) {
    for (int fd : m_fds) {
      kick_wd(fd);
    }

    if (!initial_tick) {
      // Reschedule timer.
      m_timer->expires_at(m_timer->expires_at() + boost::posix_time::seconds(TIMEOUT));
    }

    m_timer->async_wait(boost::bind(&WatchdogPlugin::tick, this, false));
  }

  void kick_wd(int fd) {
    if (write(fd, "*", 1) != 1) {
      std::cerr << "Failed to kick watchdog: " << std::strerror(errno) << std::endl;
    }
  }

  boost::asio::deadline_timer *m_timer;
  std::list<int> m_fds;
};

REGISTER_PLUGIN(WatchdogPlugin)
