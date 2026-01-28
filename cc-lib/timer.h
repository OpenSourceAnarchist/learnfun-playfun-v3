
#include <ctime>

// Extremely simple timer. Only records one start-stop span,
// and only in seconds.
struct Timer {
  Timer()  {
    Start();
  }

  void Start() {
    starttime = time(nullptr);
  }

  void Stop() {
    stoptime = time(nullptr);
  }

  auto Seconds() -> time_t {
    Stop();
    return stoptime - starttime;
  }

 private:
  time_t starttime{0}, stoptime{0};
};
