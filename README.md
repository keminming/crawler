crawler
=======
part 1:
This part build a simple http client using winsock.

part 2:
This part of the homework builds on the previous version by constructing a simple web crawler. You are welcome to use STL and the IRL HTML parser (found on beefybox in C:\463) as this will significantly reduce the development cycle. In order to construct a working crawler, you will need to separate the HTTP header from the message body, parse the former to detect the status code and redirect location, parse the latter to obtain HTML links, and finally insert those that are unique back into the crawl queue. Note that you must follow 301/302 redirects, which is accom-plished by pushing the “Location:” field into the front (not back) of the queue. The HTML parser provides only http:// links and ignores everything else, but you will need to write a similar fil-ter for redirects.

part 3:
This part of the homework scales the previous implementation to perform DNS lookups concur-rently with crawling, check all pending URLs against robots.txt, run multiple crawling and DNS threads, and shuffle the frontier (yet uncrawled pages) through an IP heap that enforces a 10-second return delay to each visited server.

web crawler
