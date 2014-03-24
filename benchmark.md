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
    ab -c 32 -n 10000 http://localhost/8080/lipsum100k.txt

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

With epoll
----------

... to be implemented!
