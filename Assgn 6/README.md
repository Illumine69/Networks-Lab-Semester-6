## How to run:

### 1. Server:

```
$ make server
gcc -Wall -o DNSserver simDNSServer.c
$ sudo ./DNSserver <Client's IP Address>
```

### 2. Client:

```
$ make client
gcc -Wall -o DNSclient simDNSClient.c
$ sudo ./DNSclient <Server's IP Address>
```

#### To get System's IP Address:

```
$ ifconfig
```

Check for `inet addr` in `eno*` or `wlo*` interface whatever you are using.  
You might need to install `ifconfig` if it's not installed:

```
$ sudo apt-get install net-tools
```
Though tested, it might happen that different interfaces for server and client would cause problem. Hence, it is recommended to run both on either wifi or ethernet.
