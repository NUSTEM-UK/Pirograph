import socket

s = socket.socket(socket.AF_INET, socket.SOCK_RAW)
host = '10.0.1.10'
port = 1881
s.connect((host, int(port)))

fh = s.makefile()

line = fh.readline()
while line.strip() != '':
    parts = line.split(':')
    if len(parts) > 1 and parts[0].lower() == 'content-type':
        # Extract boundary string from content-type
        content_type = parts[1].strip()
        boundary = content_type.split(";")[1].split('=')[1]
    line = fh.readline()

if not boundary:
    raise Exception("Can't find content-type")

# Seek ahead to first chunk
while line.strip() != boundary:
    line = fh.readline()

# Read in chunk headers
while line.strip() != '':
    parts = line.split(':')
    if len(parts) > 1 and parts[0].lower() == 'content-length':
        # Grab chunk length
        length = int(parts[1].strip())
    line = fh.readline()

img = Image.read(fh.read(length))

img.show()