#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/asio.hpp>
#include <list>

#define CONFIG_FILE "/etc/whynot.conf"
#define PLUGINS_DIR "/usr/lib/whynot/"

int main(int argc, char *argv[]) {
  boost::property_tree::ptree pt;
  try {
    boost::property_tree::ini_parser::read_ini(CONFIG_FILE, pt);
  } catch (const std::exception& ex) {
    std::cerr << "Error reading configuration file: " << ex.what() << std::endl;
    return 1;
  }

  std::list<std::string> plugins;

  try {
    std::string all_plugins(pt.get<std::string>("plugins.plugins"));
    std::stringstream s(all_plugins);
    std::string plugin;

    while (s) {
      s >> plugin;
      plugins.push_back(plugin);
    }
  } catch (const std::exception& ex) {
    std::cerr << "Error parsing configuration file: " << ex.what() << std::endl;
    return 1;
  }

  boost::asio::io_service io_service;
  boost::asio::io_service::work work(io_service);

  // OK, Now we can load
  for (std::string plugin : plugins) {
    
  }

  // Now we can run:
  try {
    io_service.run();
  } catch (const std::exception& ex) {
    std::cerr << "Error starting: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}
