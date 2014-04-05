Benchmark results
=================

Using read()/write()
--------------------

Up to and including commit `413cc7c8d7392eb98d4f643b9bea0eefa165b41a`, Curly
just handled one request synchronously before moving on to the next one.

The maximum socket backlog is 32, so we can issue at most 32 concurrent
requests if we want to be sure nothing gets refused.

Method:

	src/curly -r test_root
    ab -c 32 -n 10000 http://localhost:8080/lipsum100k.txt

Output:

    Concurrency Level:      32
	Time taken for tests:   3.773 seconds
	Complete requests:      10000
	Failed requests:        0
	Write errors:           0
	Total transferred:      1024190000 bytes
	HTML transferred:       1024000000 bytes
	Requests per second:    2650.22 [#/sec] (mean)
	Time per request:       12.074 [ms] (mean)
	Time per request:       0.377 [ms] (mean, across all concurrent requests)
	Transfer rate:          265070.73 [Kbytes/sec] received

	Connection Times (ms)
				  min  mean[+/-sd] median   max
	Connect:        0    0   0.3      0       6
	Processing:     2   12   1.4     12      28
	Waiting:        1   11   1.1     11      21
	Total:          4   12   1.4     12      28

	Percentage of the requests served within a certain time (ms)
	  50%     12
	  66%     12
	  75%     12
	  80%     12
	  90%     13
	  95%     14
	  98%     15
	  99%     18
	 100%     28 (longest request)

Using sendfile()
----------------

`sendfile()` is a Linux-specific system call to copy data from one file
descriptor into another without going through userland. Using that makes things
significantly faster:

	Concurrency Level:      32
	Time taken for tests:   2.493 seconds
	Complete requests:      10000
	Failed requests:        0
	Write errors:           0
	Total transferred:      1024190000 bytes
	HTML transferred:       1024000000 bytes
	Requests per second:    4011.20 [#/sec] (mean)
	Time per request:       7.978 [ms] (mean)
	Time per request:       0.249 [ms] (mean, across all concurrent requests)
	Transfer rate:          401194.35 [Kbytes/sec] received

	Connection Times (ms)
				  min  mean[+/-sd] median   max
	Connect:        0    1   0.2      1       3
	Processing:     3    7   0.6      7      12
	Waiting:        0    1   0.4      1       5
	Total:          4    8   0.5      8      13

	Percentage of the requests served within a certain time (ms)
	  50%      8
	  66%      8
	  75%      8
	  80%      8
	  90%      8
	  95%      9
	  98%      9
	  99%     10
	 100%     13 (longest request)

With epoll
----------

... to be implemented!

Comparison to nginx
===================

Rumour has it that [nginx](http://nginx.org) is one of the fastest HTTP servers
on the planet. Its architecture (using epoll) is similar to Curly's, although
it does a great deal more, like virtual hosts, redirection and reverse
proxying. Still, for serving static files, it should be an apples-to-apples
comparison.

Using `nginx.conf` as supplied, and the command line documented therein, running

    ab -c 256 -n 100000 http://localhost:8080/lipsum100k.txt

gives us:

	Concurrency Level:      256
	Time taken for tests:   26.900 seconds
	Complete requests:      100000
	Failed requests:        0
	Write errors:           0
	Total transferred:      10261500000 bytes
	HTML transferred:       10240000000 bytes
	Requests per second:    3717.43 [#/sec] (mean)
	Time per request:       68.865 [ms] (mean)
	Time per request:       0.269 [ms] (mean, across all concurrent requests)
	Transfer rate:          372523.10 [Kbytes/sec] received

	Connection Times (ms)
				  min  mean[+/-sd] median   max
	Connect:        2   10  52.0      8    1008
	Processing:    25   58   6.2     60     260
	Waiting:        2    8   3.4      8     217
	Total:         34   69  52.3     69    1070

	Percentage of the requests served within a certain time (ms)
	  50%     69
	  66%     70
	  75%     70
	  80%     70
	  90%     70
	  95%     71
	  98%     73
	  99%     75
	 100%   1070 (longest request)

The same benchmark for Curly yields:

	Concurrency Level:      256
	Time taken for tests:   28.956 seconds
	Complete requests:      100000
	Failed requests:        0
	Write errors:           0
	Total transferred:      10241900000 bytes
	HTML transferred:       10240000000 bytes
	Requests per second:    3453.55 [#/sec] (mean)
	Time per request:       74.127 [ms] (mean)
	Time per request:       0.290 [ms] (mean, across all concurrent requests)
	Transfer rate:          345418.89 [Kbytes/sec] received

	Connection Times (ms)
				  min  mean[+/-sd] median   max
	Connect:        2   15  77.9      8    1012
	Processing:    16   59   8.5     60     272
	Waiting:        2    7   3.6      7     222
	Total:         29   73  78.8     69    1274

	Percentage of the requests served within a certain time (ms)
	  50%     69
	  66%     72
	  75%     73
	  80%     74
	  90%     75
	  95%     76
	  98%     86
	  99%    101
	 100%   1274 (longest request)

So on this crappy benchmark, performance is certainly comparable to nginx.
