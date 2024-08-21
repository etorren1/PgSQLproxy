# PostgreSQL proxy server <a name="top"></a>
This is a simple proxy server for PostgreSQL database that logs queries.

## Quic start on Linux

`make && ./proxy`

## Configuration

Two ways to configure proxy:
* At command line startup<br />
`Example: ./proxy --server-host localhost --sp 11000 -dh 10.4.0.109 --database-port 5435`
<img src="https://github.com/etorren1/PgSQLproxy/blob/main/images/cmd.jpg" width="60%"/>

* With config file<br />
 Also it's possible to specify path to config<br />
`Example: ./proxy --config PATH`
<img src="https://github.com/etorren1/PgSQLproxy/blob/main/images/config.jpg" width="50%"/>

## Test

Tested with sysbench:
<pre>sysbench \
--db-driver=pgsql \
--report-interval=2 \
--oltp-table-size=100000 \
--oltp-tables-count=24 \
--threads=90 \
--time=720 \
--pgsql-host=localhost \
--pgsql-port=4242 \
--pgsql-user=sbtest \
--pgsql-password=password \
--pgsql-db=sbtest \
/usr/share/sysbench/tests/include/oltp_legacy/oltp.lua \
run</pre>

## Developers
[etorren](https://github.com/etorren1)  
***
<a href="#top">Gotop</a>
