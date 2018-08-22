#include <fcntl.h>
#include <poll.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  const auto env_core = getenv("ALPIDE_TEST_CORE");
  if ((env_core == nullptr) || (strcmp(env_core, "0") != 0)) {
    struct rlimit core_limit;
    getrlimit(RLIMIT_CORE, &core_limit);
    core_limit.rlim_cur = core_limit.rlim_max;
    setrlimit(RLIMIT_CORE, &core_limit);
  }

  const auto env_log  = getenv("ALPIDE_TEST_LOG");
  bool       log_fork = (env_log == nullptr) || (strcmp(env_log, "0") != 0);

  int pipefd_out[2], pipefd_err[2];
  if (pipe(pipefd_out) != 0) log_fork = false;
  if (pipe(pipefd_err) != 0) log_fork = false;

  int pid = -1;
  if (log_fork) pid = fork();

  if (pid == 0) {
    close(pipefd_out[0]);
    close(pipefd_err[0]);
    dup2(pipefd_out[1], 1);
    dup2(pipefd_err[1], 2);
    close(pipefd_out[1]);
    close(pipefd_err[1]);
  }

  if (pid <= 0) {
    QCoreApplication::setOrganizationName("ALICE");
    QCoreApplication::setOrganizationDomain("alice.cern");
    QCoreApplication::setApplicationName("Alpide Testing");

    QApplication a(argc, argv);
    MainWindow   w;
    w.show();

    return a.exec();
  }

  if (pid > 0) {
    close(pipefd_out[1]);
    close(pipefd_err[1]);

    const std::time_t t = time(nullptr);
    char              time_suffix[50];
    strftime(time_suffix, sizeof(time_suffix), "%y%m%d_%H%M%S", std::localtime(&t));
    std::string   logfilename = std::string("gui_") + time_suffix + ".log";
    int           fd          = open(logfilename.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
    char          buffer[1024];
    int           nbytes;
    struct pollfd fds[2];
    fds[0].fd     = pipefd_out[0];
    fds[0].events = POLLIN | POLLHUP;
    fds[1].fd     = pipefd_err[0];
    fds[1].events = POLLIN | POLLHUP;
    int ret;
    while ((ret = poll(fds, 2, -1))) {
      if (fds[0].revents & POLLHUP) break;
      if (fds[1].revents & POLLHUP) break;
      if (fds[0].revents != 0) {
        if ((nbytes = read(pipefd_out[0], buffer, sizeof(buffer))) > 0) {
          if (write(STDOUT_FILENO, buffer, nbytes) != nbytes)
            printf("error writing stdout to stdout\n");
          if (write(fd, buffer, nbytes) != nbytes) printf("error writing stdout to file\n");
        }
      }
      if (fds[1].revents != 0) {
        if ((nbytes = read(pipefd_err[0], buffer, sizeof(buffer))) > 0) {
          if (write(STDERR_FILENO, buffer, nbytes) != nbytes)
            printf("error writing stderr to stderr\n");
          if (write(fd, buffer, nbytes) != nbytes) printf("error writing stderr to file\n");
        }
      }
    }

    wait(NULL);

    close(pipefd_out[0]);
    close(pipefd_err[0]);
    close(fd);
  }
}
