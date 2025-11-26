import pytest
import os
import subprocess
import time
import signal

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
EXECUTABLE = os.path.join(BASE_DIR, "../httpd")
PID_FILE = "/tmp/test_httpd_daemon.pid"
LOG_FILE = "/tmp/test_httpd_daemon.log"

def is_process_running(pid):
    try:
        os.kill(pid, 0)
    except OSError:
        return False
    return True

@pytest.fixture
def daemon_env():
    base_cmd = [
        EXECUTABLE,
        "--server_name", "test_daemon",
        "--port", "8082",
        "--ip", "127.0.0.1",
        "--root_dir", ".",
        "--pid_file", PID_FILE,
        "--log_file", LOG_FILE
    ]

    # Clean
    if os.path.exists(PID_FILE):
        subprocess.run(base_cmd + ["--daemon", "stop"], stderr=subprocess.DEVNULL)

    yield base_cmd

    # Clean after test
    if os.path.exists(PID_FILE):
        subprocess.run(base_cmd + ["--daemon", "stop"], stderr=subprocess.DEVNULL)

    if os.path.exists(LOG_FILE):
        os.remove(LOG_FILE)

def test_daemon_lifecycle(daemon_env):
    base_cmd = daemon_env

    # Start
    subprocess.run(base_cmd + ["--daemon", "start"], check=True)
    time.sleep(0.5)

    assert os.path.exists(PID_FILE)

    with open(PID_FILE, "r") as f:
        pid_1 = int(f.read().strip())

    assert is_process_running(pid_1)

    # Restart
    subprocess.run(base_cmd + ["--daemon", "restart"], check=True)
    time.sleep(0.5)

    assert os.path.exists(PID_FILE)

    with open(PID_FILE, "r") as f:
        pid_2 = int(f.read().strip())

    assert is_process_running(pid_2)
    assert pid_1 != pid_2
    assert not is_process_running(pid_1)

    # Stop
    subprocess.run(base_cmd + ["--daemon", "stop"], check=True)
    time.sleep(0.5)

    assert not os.path.exists(PID_FILE)
    assert not is_process_running(pid_2)

def test_daemon_double_start(daemon_env):
    base_cmd = daemon_env

    # First start
    subprocess.run(base_cmd + ["--daemon", "start"], check=True)
    time.sleep(0.5)

    with open(PID_FILE, "r") as f:
        pid_orig = int(f.read().strip())

    # Second start
    subprocess.run(base_cmd + ["--daemon", "start"], check=False)
    time.sleep(0.5)

    with open(PID_FILE, "r") as f:
        pid_new = int(f.read().strip())

    assert pid_orig == pid_new
