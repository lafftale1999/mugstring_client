#!/bin/python3
import ftplib
import os

HOST_ADDRESS    = '192.168.1.100'
USERNAME        = 'root'
PASSWORD        = 'admin'
TIMEOUT_S       = 10

BINARIES_DIR    = '/home/lafftale/Development/pilots/MugstringVPN/filesystem-overlay/usr/local/sbin'
def upload_file():
    binaries = os.listdir(BINARIES_DIR)
    print(binaries)

    with ftplib.FTP(host=HOST_ADDRESS, timeout=TIMEOUT_S) as ftp:
        ftp.login()
        ftp.cwd('upload')

        for bin in binaries:
            with open(f'{BINARIES_DIR}/{bin}', 'rb') as f:
                ftp.storbinary(f'STOR {bin}', f)
            



def main():
    upload_file()

if __name__ == '__main__':
    main()
