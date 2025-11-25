import requests
import socket
import pytest
import os

def test_404_not_found(server):
    resp = requests.get(f"{server['url']}/filedoesntexist.html")
    assert resp.status_code == 404

# Try to use an unimplemented method
def test_405_bad_method(server):
    resp = requests.post(f"{server['url']}/index.html", data={"foo": "bar"})
    assert resp.status_code == 405

# Try to send without valid HTTP
def test_bad_request_line(server):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((server['ip'], server['port']))
    payload = b"GIBBERISH / HTTP/1.1\r\nHost: localhost\r\n\r\n"
    s.sendall(payload)
    response = s.recv(1024)
    s.close()
    decoded = response.decode(errors='replace')

    assert "405" in decoded

def test_missing_headers(server):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((server['ip'], server['port']))
    payload = b"GET / HTTP/1.1\r\n\r\n"
    s.sendall(payload)
    response = s.recv(1024)
    s.close()
    decoded = response.decode(errors='replace')
    assert "404" in decoded

def test_403_forbidden(server):
    filename = "secret.txt"
    with open(filename, "w") as f:
        f.write("Top Secret Data")
    os.chmod(filename, 0o000)
    try:
        resp = requests.get(f"{server['url']}/{filename}")
        assert resp.status_code == 403
    finally:
        os.chmod(filename, 0o644)
        os.remove(filename)
