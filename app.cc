#include <unistd.h>
#include <errno.h>
#include "app.hh"
#include "log.hh"

bool App::app_to_exist = false;

App::App(const std::string& app_name) : m_name(app_name)
{
    m_status = E_APP_STATUS_NEW;
}

bool App::Init(int argc, char *argv[])
{
    PR_TRACE("Initializing app=[%s]", name().c_str());
    bool rc = do_init(argc, argv);
    if ( ! rc ) {
        PR_DEBUG("failed to init app=[%s]", name().c_str());
    }
    m_status = E_APP_STATUS_INITED;
    return rc;
}

bool App::Start()
{
    PR_TRACE("Starting app=[%s]", name().c_str());
    bool rc = do_start();
    if ( ! rc ) {
        PR_DEBUG("failed to start app=[%s]", name().c_str());
    }
    m_status = E_APP_STATUS_STARTED;
    return rc;
}

bool App::Finish()
{
    PR_TRACE("Finishing app=[%s]", name().c_str());
    bool rc = do_finish();
    if ( ! rc ) {
        PR_DEBUG("failed to finish app=[%s]", name().c_str());
    }
    m_status = E_APP_STATUS_FINISHED;
    return rc;
}

bool App::Stop()
{
    PR_TRACE("Stopping app=[%s]", name().c_str());
    bool rc = do_stop();
    if ( ! rc ) {
        PR_DEBUG("failed to stop app=[%s]", name().c_str());
    }
    m_status = E_APP_STATUS_STOPPED;
    return rc;
}

e_app_status App::status() const
{
    return m_status;
}
const std::string& App::name() const
{
    return m_name;
}
void App::setName(const std::string &app_name)
{
    m_name = app_name;
}

bool App::do_fork()
{
    // failed
    if ( 0 != daemon(1, 0) ) {
        PR_ERROR("failed to call daemon, errno=%d", errno);
        return false;
    }
    return true;
}
