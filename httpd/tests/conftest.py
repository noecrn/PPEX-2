import pytest
import socket
import time
import subprocess
import os
import sys
import requests

HOST = "127.0.0.1"
PORT = "8080"

# The path to the exectutable
EXECUTABLE = os.path.abspath(os.path.join(os.path.dirname(__file__), "../httpd"))

def wait_for_port(host, port, timeout=2.0):
    start_time = time.time()
    while time.time() - start_time < timeout:
        try:
            with socket.create_connection((host, port), timeout=0.1):
                return True
        except (OSError, ConnectionRefusedError):
            time.sleep(0.05)
    raise RuntimeError(f"The server doesn't launch on: {host}:{port}")

@pytest.fixture(scope="function")
def server():
    stdout_file = open("server_out.log", "w")

    cmd = [
        EXECUTABLE,
        "--port", PORT,
        "--ip", HOST,
        "--server_name", "localhost",
        "--root_dir", ".",
        "--pid_file", "/tmp/test_httpd.pid"
    ]

    # Launch the process
    proc = subprocess.Popen(cmd, stdout=stdout_file, stderr=subprocess.PIPE, cwd=os.getcwd())

    try:
        wait_for_port(HOST, int(PORT))
    except RuntimeError:
        print("\n[ERREUR] Impossible to launch the server !")
        print(proc.stderr.read().decode(errors="replace"))
        proc.kill()
        pytest.fail("The server failed at the launch")

    # Return info to tests
    server_info = {
        "url": f"http://{HOST}:{PORT}",
        "ip": HOST,
        "port": int(PORT)
    }

    yield server_info 

    # Clean
    proc.terminate()
    try:
        proc.wait(timeout=1)
    except subprocess.TimeoutExpired:
        proc.kill()

    stdout_file.close()
    if os.path.exists("/tmp/test_httpd.pid"):
        try:
            os.remove("/tmp/test_httpd.pid")
        except:
            pass
