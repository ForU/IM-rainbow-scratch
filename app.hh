#ifndef INCLUDE_APP_HPP
#define INCLUDE_APP_HPP

#include <string>

enum e_app_status
{
    E_APP_STATUS_NEW,
    E_APP_STATUS_INITED,
    E_APP_STATUS_STARTED,
    E_APP_STATUS_STOPPED,
    E_APP_STATUS_FINISHED,
};

class App
{
public:
    // TODO: [2013-12-08] to init signal 
    App(const std::string& app_name="Application");
    ~App() {}

    bool Init(int argc, char *argv[]);
    bool Start();
    bool Finish();
    bool Stop();

    e_app_status status() const;
    const std::string& name() const;
    void setName(const std::string &app_name);

protected:
    virtual bool do_init(int argc, char *argv[]) = 0;
    virtual bool do_start() = 0;
    virtual bool do_stop() = 0;
    virtual bool do_finish() = 0;
protected:
     bool do_fork();

protected:
    static bool app_to_exist;

private:
    std::string m_name;
    e_app_status m_status;
};

#endif /* INCLUDE_APP_HPP */
