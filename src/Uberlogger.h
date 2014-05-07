#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <functional>
#include <mutex>

// TODO:
//  1. Disable/enable threadsafety
//  2. Log levels
//  3. COLORS! :3

#define UBERDEFAULT "default"

class Uberlogger {
public:


  class Logger {
  private:
    std::string  _logname;
    std::string  _template;
    std::map<std::string, std::function<std::string(void)>> replace;
    std::mutex omutex;

  public:
    Logger(std::string logname = UBERDEFAULT) {
      templ("[%name] %msg");
      _logname = logname;
      repl("%name", [=]() { return _logname; });
    }

    Logger(const Logger& logger) {
      _logname = logger._logname;
    }

    void add (std::string msg) {
      //Need to fix
      omutex.lock();
      std::string result = _template;
      omutex.unlock();

      for(auto r : replace) {
        auto begin = result.find(r.first);
        if(begin != std::string::npos)
          result.replace(begin, r.first.size(), r.second());
      }

      auto begin = result.find("%msg");
      if(begin != std::string::npos)
        result.replace(begin, 4, msg);

      begin = result.find("%name");
      if(begin != std::string::npos)
        result.replace(begin, 5, _logname);

      omutex.lock();
      std::cout << result << std::endl;
      omutex.unlock();
    }

    void templ(const std::string& _t) {
      omutex.lock();
      _template = _t;
      omutex.unlock();
    }

    void repl(const std::string& str, std::function<std::string(void)> func) {
      omutex.lock();
      replace.emplace(str, func);
      omutex.unlock();
    }
  };

  static Logger& instance(const std::string& logname = UBERDEFAULT) {
    auto logger = loggers.find(logname);
    if(logger == loggers.end()) {
      loggers.emplace(logname, Logger(logname));
    }
    return loggers[logname];
  }

private:
  static std::map<std::string, Logger> loggers;
};

class Message {
private:
  std::stringstream msg;
  std::string logname;
  bool end;

public:
  class End {};

  Message(const Message& m) {
    end = m.end;
    logname = m.logname;
    msg << m.msg.str();
  }

  Message(const End&) {
    end = true;
  }

  template<class T>
  Message(const T& some) {
    logname = UBERDEFAULT;
    msg << some;
    end = false;
  }

  Message() {
    end = false;
    logname = UBERDEFAULT;
  }

  Message& operator<< (const Message& some) {
    if(some.end) {
      Uberlogger::instance(logname).add(this->get());
    } else {
      msg << some.msg.str();
    }
    return *this;
  }

  std::string get() const {
    return msg.str();
  }

  Message& withLogname(std::string _ln) {
    logname = _ln;
    return *this;
  }
};

std::map<std::string, Uberlogger::Logger> Uberlogger::loggers;

static inline Message UBERLOG() {
  return Message();
}

static inline Message UBERLOG(const std::string& s) {
  return Message().withLogname(s);
}

static inline Message UBEREND() {
  return Message::End();
}

static inline void UBERTEMPLATE(const std::string& logname, const std::string& tmpl) {
  Uberlogger::instance(logname).templ(tmpl);
}

static inline void UBERTEMPLATE(const std::string& tmpl) {
  Uberlogger::instance().templ(tmpl);
}

static inline void UBERREPLACE(const std::string& x, std::function<std::string(void)> y) {
  Uberlogger::instance().repl(x, y);
}

static inline void UBERREPLACE(const std::string& logname, const std::string& x, std::function<std::string(void)> y) {
  Uberlogger::instance(logname).repl(x, y);
}
