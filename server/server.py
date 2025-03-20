#!/usr/bin/env python3
import os
import sys
import signal
import random
import logging
import json
from flask import Flask, request, abort, Response

app = Flask(__name__)

# --- Configuration ---
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
QML_DIR = os.path.join(BASE_DIR, "qml_files")
PORT_RANGE = (10000, 20000)

# --- Logging Setup ---
logging.basicConfig(level=logging.DEBUG, format="[%(asctime)s] %(levelname)s: %(message)s")
logger = logging.getLogger("SimpleServer")

# --- Helper Function ---
def get_qml_content(filename):
    """
    Reads and returns the content of a QML file located in QML_DIR.
    Path traversal is prevented by using the basename of the filename.
    """
    safe_filename = os.path.basename(filename)
    file_path = os.path.join(QML_DIR, safe_filename)
    if not os.path.exists(file_path):
        logger.error("QML file not found: %s", file_path)
        abort(404, description="QML file not found")
    with open(file_path, "r") as f:
        content = f.read()
        logger.debug("QML file '%s' read successfully.", safe_filename)
        return content

# --- Application ---
def create_app():
    """
    Create and configure the Flask application.
    """
    app = Flask(__name__)

    @app.route("/api/v1/getQML", methods=["POST"])
    def get_qml():
        logger.info("Received request to get QML file.")
        data = request.get_json()
        if not data or "filename" not in data:
            logger.error("Missing 'filename' in JSON data")
            abort(400, description="Missing 'filename' in JSON data")
        content = get_qml_content(data["filename"])
        logger.info("Sending QML file content for '%s'", data["filename"])
        return Response(content, status=200, mimetype="text/plain")

    @app.route("/api/v1/shutdown", methods=["POST"])
    def shutdown():
        logger.info("Received shutdown request")
        os.kill(os.getpid(), signal.SIGINT)
        return "Server shutting down...", 200

    @app.route("/api/v1/cleanup", methods=["POST"])
    def cleanup():
        logger.info("Received cleanup request.")
        logger.info("Cleanup initiated. Preparing to shutdown server for cleanup.")
        # Attempt to retrieve and call the shutdown function for cleanup
        func = request.environ.get("werkzeug.server.shutdown")
        if func is None:
            logger.error("Server shutdown function not available during cleanup")
            abort(500, description="Server shutdown function not available")
        logger.info("Server shutdown initiated for cleanup.")
        func()
        logger.info("Cleanup complete. Server shutdown function executed.")
        return "Cleanup initiated. Server will shutdown and perform cleanup.", 200

    return app

# --- Server Class ---
class SimpleServer:
    def __init__(self):
        self.port = random.randint(PORT_RANGE[0], PORT_RANGE[1])
        self.app = create_app()
        logger.info("Server initialized with port %d", self.port)

    def run(self):
        logger.info("Starting server on port: %d", self.port)
        print(f"::SERVER_PORT::{self.port}", flush=True)  # Add special marker
        self.app.run(host="127.0.0.1", port=self.port, use_reloader=False)

# --- Main Entry Point ---
def main():
    server = SimpleServer()
    server.run()

if __name__ == "__main__":
    main()
