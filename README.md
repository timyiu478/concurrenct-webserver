# Concurrent Web Server in C

This project implements a simple concurrent web server in C, designed to handle multiple requests simultaneously using multithreading. The server is capable of serving static files and executing CGI scripts.

## Usage

```bash
$ ./wserver -d <directory> -p <port> -t <threads> -b <max_connections>
```

## Test the multithreading  with `httperf` and `spin.cgi`

We use `httperf` to simulate multiple clients making requests to the server. Since the `spin.cgi` script is designed to take a 1 second pause, we can use it to test the server's ability to handle concurrent requests. We test the following server configurations:

1. 1000 connections with 100 threads
2. 1000 connections with 100 threads

We expect the config 1 to handle the requests efficiently, while config 2 should struggle due to the limited number of threads.

### Config 1: 100 Threads, 1000 Max Connections

```bash
$ ./wserver -d . -p 8000 -t 50 -b 1000
$ httperf --server 0.0.0.0 --port 8000 --uri "/spin.cgi?1" --num-conns 1000 --rate 100
httperf: warning: open file limit > FD_SETSIZE; limiting max. # of open files to FD_SETSIZE
Maximum connect burst length: 1

Total: connections 1000 requests 1000 replies 1000 test-duration 11.044 s

Connection rate: 90.5 conn/s (11.0 ms/conn, <=106 concurrent connections)
Connection time [ms]: min 1003.7 avg 1028.8 max 1067.6 median 1028.5 stddev 15.6
Connection time [ms]: connect 0.0
Connection length [replies/conn]: 1.000

Request rate: 90.5 req/s (11.0 ms/req)
Request size [B]: 70.0

Reply rate [replies/s]: min 79.8 avg 89.5 max 99.2 stddev 13.7 (2 samples)
Reply time [ms]: response 24.4 transfer 1004.4
Reply size [B]: header 90.0 content 127.0 footer 0.0 (total 217.0)
Reply status: 1xx=0 2xx=1000 3xx=0 4xx=0 5xx=0

CPU time [s]: user 1.64 system 9.13 (user 14.9% system 82.7% total 97.5%)
Net I/O: 25.4 KB/s (0.2*10^6 bps)

Errors: total 0 client-timo 0 socket-timo 0 connrefused 0 connreset 0
Errors: fd-unavail 0 addrunavail 0 ftab-full 0 other 0
```

### Config 2: 10 Threads, 1000 Max Connections

```bash
$ ./wserver -d . -p 8000 -t 10 -b 1000
$ httperf --server 0.0.0.0 --port 8000 --uri "/spin.cgi?1" --num-conns 1000 --rate 100
httperf --client=0/1 --server=0.0.0.0 --port=8000 --uri=/spin.cgi?1 --rate=100 --send-buffer=4096 --recv-buffer=16384 --num-conns=1000 --num-calls=1
httperf: warning: open file limit > FD_SETSIZE; limiting max. # of open files to FD_SETSIZE
Maximum connect burst length: 1

Total: connections 1000 requests 1000 replies 1000 test-duration 100.530 s

Connection rate: 9.9 conn/s (100.5 ms/conn, <=910 concurrent connections)
Connection time [ms]: min 1003.7 avg 45783.3 max 90584.6 median 45346.5 stddev 26125.2
Connection time [ms]: connect 0.0
Connection length [replies/conn]: 1.000

Request rate: 9.9 req/s (100.5 ms/req)
Request size [B]: 70.0

Reply rate [replies/s]: min 8.0 avg 9.9 max 10.0 stddev 0.4 (20 samples)
Reply time [ms]: response 44779.5 transfer 1003.8
Reply size [B]: header 90.0 content 127.0 footer 0.0 (total 217.0)
Reply status: 1xx=0 2xx=1000 3xx=0 4xx=0 5xx=0

CPU time [s]: user 6.38 system 93.73 (user 6.3% system 93.2% total 99.6%)
Net I/O: 2.8 KB/s (0.0*10^6 bps)

Errors: total 0 client-timo 0 socket-timo 0 connrefused 0 connreset 0
Errors: fd-unavail 0 addrunavail 0 ftab-full 0 other 0
```

### Analysis

The config 1 is 10 times faster than config 2.
