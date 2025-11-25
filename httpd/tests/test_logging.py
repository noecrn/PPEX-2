import requests
import time
import os

def test_log_request(server):
    # Prepare the test file
    with open("hello.txt", "w") as f:
        f.write("Hello World")

    # Do the request
    requests.get(f"{server['url']}/hello.txt")

    time.sleep(0.1)

    # Read the log file
    if os.path.exists("server_out.log"):
        with open("server_out.log", "r") as f:
            logs = f.read()

        print(f"\n[DEBUG] Logs:\n{logs}")

        # Check infos
        assert "GET" in logs
        assert "/hello.txt" in logs
        assert "127.0.0.1" in logs
    else:
        assert False, "No file server_out.log"
