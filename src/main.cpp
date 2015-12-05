#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/asio.hpp>
#include <list>
#include <dlfcn.h>
#include "plugin.h"

#define CONFIG_FILE "/etc/whynot.conf"
#define PLUGINS_DIR "/usr/lib/whynot/"

typedef Plugin *(*init)();

int main(int argc, char *argv[]) {
  std::map<void *, Plugin *> loaded_plugins;

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

    while (!s.eof()) {
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
  for (std::string& plugin : plugins) {
    std::stringstream s;
    s << PLUGINS_DIR << "/lib" << plugin << ".so";
    void *handle = dlopen(s.str().c_str(), RTLD_LAZY);
    if (!handle) {
      std::cerr << "Failed to load " << plugin << " :" << dlerror() << std::endl;
      continue;
    }

    init init_func = (init) dlsym(handle, "__init");
    if (!init_func) {
      std::cerr << plugin << " is not a proper plugin (Symbol __init not found)" << std::endl;
      continue;
    }

    Plugin *p = init_func();
    if (!p->start()) {
      delete p;
      dlclose(handle);
      continue;
    }

    loaded_plugins.insert(std::make_pair(handle, p));
  }

  // Now we can run:
  try {
    io_service.run();
  } catch (const std::exception& ex) {
    std::cerr << "Error starting: " << ex.what() << std::endl;
    return 1;
  }

  for (auto& pair : loaded_plugins) {
    pair.second->stop();
    delete pair.second;
    dlclose(pair.first);
  }

  loaded_plugins.clear();

  return 0;
}
