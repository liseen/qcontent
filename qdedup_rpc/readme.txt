                                readme
                                ======

Author:  <zhenbao.zhou@g-laptop>
Date: 2011-09-06 10:42:05 CST


Table of Contents
=================
1 简介 
2 使用方法 
    2.1 启动多个server。 
        2.1.1 server的help信息 
    2.2 使用dedup client 
        2.2.1 client的help信息 
3 如何编译 
    3.1 编译依赖的包depends 
    3.2 编译client 
    3.3 编译server 
4 性能测试 
5 测试用例 
    5.1 测试server 
    5.2 测试client 


1 简介 
~~~~~~~
基于rpc 实现的dedup. 按照site将各篇文章送至各个server 进行dedup, 计算是否要dedup.
dedup算法和之前一致，使用长句子算hash。
hash算法是city_hash.city_hash的结果是uint64.因此，在dedup server里又根据mmap_file_size 大小对hash值取余.

2 使用方法 
~~~~~~~~~~~

2.1 启动多个server。 
=====================
   ./dedup_server -p 8895 -s 30 -f a_test2 -t 100 -T 100000
   ./dedup_xserver -p 8893 -s 30 -f a_test -t 100 -T 100000
   ……

2.1.1 server的help信息 
-----------------------
$ ./dedup_server -h
Usage:  ./dedup_server options 
  -h  --help             Display this usage information.
  -f                     Mmap file name. (Default is 'dedup_mmap_file') 
  -s                     Mmap file size. (Default is 100M)
  -p                     Server port.   (Default is 8891)
  -t                     Server thread num(Default is 10) 
  -D                     run as a daemon
  -T                     Sync threshold of mmap (Default is 500000) 
  

2.2 使用dedup client 
=====================
    cat dedup_file | ./dedup_client -S "localhost:8881;localhost:8891" -o output
   输出3个文件
   # output.out   通过dedup的url
   # output.del   被dedup掉的url
   # output.bad_list   列表页，但是new_link_size=0

2.2.1 client的help信息 
-----------------------
$ ./dedup_client -h
Usage:  ./dedup_client options 
  -h               Display this usage information.
  -u               Display usage of mmap file.
  -c               Config file for client. (Default dedup.conf)
  -d               Debug mode. Another format of input line
  -s               Also output Deduped url in stderr.Notice: Undeduped content always output in stdout
  -t               Stat the documents
  -S               Such as 'localhost:8891,localhost:8893,192.168.4.2:2313'
  -o               Output file's name. default (qdedup_client)
  -H               Halt dedup servers
  -v               Print verbose for every article. Otherwise, content and title will not output for deduped files

3 如何编译 
~~~~~~~~~~~

3.1 编译依赖的包depends 
========================
msgpack-rpc msgpack qcontentcommon qmake

3.2 编译client 
===============
   cd dedup_client; qmake; make

3.3 编译server 
===============
   cd dedup_server; qmake ;make
   

4 性能测试 
~~~~~~~~~~~
每秒处理500 ~ 600篇文章.

$ time cat 10000 | ./dedup_client -t > good
deduped documents:2029

real     0m3.544s
user     0m2.240s
sys     0m1.680s


5 测试用例 
~~~~~~~~~~~
cd test;

5.1 测试server 
===============
./server_test.pl


5.2 测试client 
===============
./client_test.pl

