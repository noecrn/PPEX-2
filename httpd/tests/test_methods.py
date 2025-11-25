import requests

def test_get_basic(server):
    # Prepare the test file
    with open("hello.txt", "w") as f:
        f.write("Hello World")

    # Use the given URL
    resp = requests.get(f"{server['url']}/hello.txt")

    # Check
    assert resp.status_code == 200
    assert resp.text == "Hello World"
    assert resp.headers["Content-Length"] == "11"

def test_head_method(server):
    with open("hello.txt", "w") as f:
        f.write("Hello World")

    resp = requests.head(f"{server['url']}/hello.txt")

    assert resp.status_code == 200
    assert resp.text == "" # No body for HEAD
    assert resp.headers["Content-Length"] == "11"
