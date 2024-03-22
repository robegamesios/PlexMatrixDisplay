import http.server
import socket

class MyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def address_string(self):
        # Override address_string method to return local IP address
        host, _ = self.client_address
        return host

def run_server(port=8000):
    server_address = ('', port)
    httpd = http.server.HTTPServer(server_address, MyHTTPRequestHandler)
    local_ip = socket.gethostbyname(socket.gethostname())
    print(f'Serving HTTP on {local_ip} port {port} (http://{local_ip}:{port}/) ...')
    httpd.serve_forever()

if __name__ == '__main__':
    run_server()
