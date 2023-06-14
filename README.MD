parse nginx access log into duckdb database

解析的日志格式
```
    log_format vhosts '$remote_addr - $remote_user [$time_local] "$request" '
                      '$status $body_bytes_sent "$http_referer" '
                      '"$http_user_agent" "$http_x_forwarded_for" '
                      '$host $request_length $bytes_sent $upstream_addr '
                      '$upstream_status $request_time $upstream_response_time '
                      '$upstream_connect_time $upstream_header_time';


```

日志文件解析及相关逻辑复用log2sqlite

```
main.cpp
parser.cpp
process.cpp
```
与原版 https://github.com/suconghou/log2sqlite 一致

仅db.cpp重新开发


### 编译

#### 编译duckdb静态库

下载duckdb官方`libduckdb-src.zip`源码，解压,有三个文件

`g++ -c -Wall -std=c++17 -O3 duckdb.cpp && ar -crv libduckdb.a duckdb.o` 进行编译，编译可能需要2分钟时间,需要2G以上内存

编译出 `libduckdb.a`

复制解压出的`duckdb.hpp`文件到本项目，编译本项目

#### 编译本程序

`g++ -Wall -std=c++17 -O3 -o log2duckdb /path/to/libduckdb.a main.cpp`


## 测试

与c++版本log2sqlite相比，插入速度基本相同，每秒插入10W+

但是数据库文件相比sqlite小,是其五分之一大小

查询速度比sqlite快很多

```
'SELECT count(1) as n,request FROM nginx_log GROUP BY request ORDER BY n desc LIMIT 50;' 比sqlite快12倍

'SELECT count(1) as n,remote_addr FROM nginx_log GROUP BY remote_addr ORDER BY n desc LIMIT 50;' 比sqlite快13倍

'SELECT count(1) as n,http_user_agent FROM nginx_log GROUP BY http_user_agent ORDER BY n desc LIMIT 50;' 比sqlite快28倍

'SELECT count(1) as n,request,http_user_agent FROM nginx_log GROUP BY http_user_agent,request ORDER BY n desc LIMIT 50;' 比sqlite快11倍

'SELECT count(1) as n,request,remote_addr FROM nginx_log GROUP BY remote_addr,request ORDER BY n desc LIMIT 50;' 比sqlite快12倍

'SELECT count(1) as n,request,round(avg(request_time),2) FROM nginx_log WHERE request_time > 2 GROUP BY request ORDER BY n desc LIMIT 50;' 比sqlite快6倍

"SELECT request FROM nginx_log WHERE LENGTH(request) - LENGTH(REPLACE(request, ' ', '')) <> 2;" 比sqlite快3倍

```


