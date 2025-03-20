import socket
import random
import requests
from requests.exceptions import RequestException

def find_open_port(start=10000, end=20000):
    ports = list(range(start, end + 1))
    random.shuffle(ports)  # Randomize to distribute load
    for port in ports:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(0.01)  # Reduced timeout for localhost
            if s.connect_ex(("127.0.0.1", port)) == 0:
                # Validate it's the correct server
                try:
                    response = requests.get(f"http://127.0.0.1:{port}/health", timeout=1)
                    if response.status_code == 200:
                        return port
                except RequestException:
                    continue  # Not the correct server
    return None

port = find_open_port()
if port:
    BASE_URL = f"http://127.0.0.1:{port}"
    print(f"Found server at: {BASE_URL}")

    # Test endpoints with error handling
    endpoints = [
        ("post", "/api/v1/getQML", {"filename": "main.qml"}),
        ("post", "/api/v1/shutdown", None),
        ("post", "/api/v1/cleanup", None)
    ]

    for method, path, data in endpoints:
        url = f"{BASE_URL}{path}"
        try:
            if method == "post":
                response = requests.post(url, json=data, timeout=5)
            else:
                response = requests.get(url, timeout=5)
            print(f"{method.upper()} {path} Response: {response.status_code} {response.text}")
        except RequestException as e:
            print(f"{method.upper()} {path} Failed: {str(e)}")
else:
    print("No server found in the port range.")