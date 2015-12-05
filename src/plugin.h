#ifndef __PLUGIN_H__
#define __PLUGIN_H__

class Plugin {
public:
  virtual ~Plugin() {}
  virtual bool start() = 0;
  virtual void stop() = 0;

protected:
  Plugin() {}
};

#define REGISTER_PLUGIN(x)				\
  extern "C" Plugin *__init() {				\
    return new x;					\
  }							\

#endif /* __PLUGIN_H__ */
