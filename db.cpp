#include "duckdb.hpp"
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;
using namespace duckdb;

#define CHECK(a) \
    if (a != 0)  \
    return a

class dbutil
{

private:
    DuckDB *db;
    Connection *con;
    Appender *appender;

public:
    dbutil()
    {
        this->db = new DuckDB("ngx_log.db");
        this->con = new Connection(*this->db);
        this->con->Query("DROP TABLE IF EXISTS nginx_log");
        this->con->Query("CREATE TABLE nginx_log ( time UINTEGER NOT NULL, remote_addr VARCHAR NOT NULL, remote_user VARCHAR NOT NULL, request VARCHAR NOT NULL, status USMALLINT NOT NULL, body_bytes_sent UINTEGER NOT NULL, http_referer VARCHAR NOT NULL, http_user_agent VARCHAR NOT NULL, http_x_forwarded_for VARCHAR NOT NULL, host VARCHAR NOT NULL, request_length UINTEGER NOT NULL, bytes_sent UINTEGER NOT NULL, upstream_addr VARCHAR NOT NULL, upstream_status USMALLINT NOT NULL, request_time REAL NOT NULL, upstream_response_time REAL NOT NULL, upstream_connect_time REAL NOT NULL, upstream_header_time REAL NOT NULL )");
    }

    int begin()
    {
        this->appender = new Appender(*this->con, "nginx_log");
        return 0;
    }

    // 如果出错将会抛出错误，否则必然返回0
    int insert_log(unsigned int time, string &remote_addr, string &remote_user, string &request, unsigned int status, unsigned int body_bytes_sent, string &http_referer, string &http_user_agent, string &http_x_forwarded_for, string &host, unsigned int request_length, unsigned int bytes_sent, string &upstream_addr, unsigned int upstream_status, float request_time, float upstream_response_time, float upstream_connect_time, float upstream_header_time)
    {
        this->appender->AppendRow(time, remote_addr.c_str(), remote_user.c_str(), request.c_str(), status, body_bytes_sent, http_referer.c_str(), http_user_agent.c_str(), http_x_forwarded_for.c_str(), host.c_str(), request_length, bytes_sent, upstream_addr.c_str(), upstream_status, request_time, upstream_response_time, upstream_connect_time, upstream_header_time);
        return 0;
    }

    int end()
    {
        this->appender->Close();
        return 0;
    }
};

namespace duckdb
{
    enum class RenderMode : uint8_t
    {
        ROWS,
        COLUMNS
    };

    struct BoxRendererConfig
    {
        // a max_width of 0 means we default to the terminal width
        idx_t max_width = 0;
        // the maximum amount of rows to render
        idx_t max_rows = 20;
        // the limit that is applied prior to rendering
        // if we are rendering exactly "limit" rows then a question mark is rendered instead
        idx_t limit = 0;
        // the max col width determines the maximum size of a single column
        // note that the max col width is only used if the result does not fit on the screen
        idx_t max_col_width = 20;
        //! how to render NULL values
        string null_value = "NULL";
        //! Whether or not to render row-wise or column-wise
        RenderMode render_mode = RenderMode::ROWS;

#ifndef DUCKDB_ASCII_TREE_RENDERER
        const char *LTCORNER = "\342\224\214"; // "┌";
        const char *RTCORNER = "\342\224\220"; // "┐";
        const char *LDCORNER = "\342\224\224"; // "└";
        const char *RDCORNER = "\342\224\230"; // "┘";

        const char *MIDDLE = "\342\224\274";  // "┼";
        const char *TMIDDLE = "\342\224\254"; // "┬";
        const char *LMIDDLE = "\342\224\234"; // "├";
        const char *RMIDDLE = "\342\224\244"; // "┤";
        const char *DMIDDLE = "\342\224\264"; // "┴";

        const char *VERTICAL = "\342\224\202";   // "│";
        const char *HORIZONTAL = "\342\224\200"; // "─";

        const char *DOTDOTDOT = "\xE2\x80\xA6"; // "…";
        const char *DOT = "\xC2\xB7";           // "·";
        const idx_t DOTDOTDOT_LENGTH = 1;

#else
        // ASCII version
        const char *LTCORNER = "<";
        const char *RTCORNER = ">";
        const char *LDCORNER = "<";
        const char *RDCORNER = ">";

        const char *MIDDLE = "+";
        const char *TMIDDLE = "+";
        const char *LMIDDLE = "+";
        const char *RMIDDLE = "+";
        const char *DMIDDLE = "+";

        const char *VERTICAL = "|";
        const char *HORIZONTAL = "-";

        const char *DOTDOTDOT = "..."; // "...";
        const char *DOT = ".";         // ".";
        const idx_t DOTDOTDOT_LENGTH = 3;
#endif
    };
}

int db_query(const char *file, const char *sql)
{
    DBConfig config;
    config.options.access_mode = getenv("RW") ? AccessMode::READ_WRITE : AccessMode::READ_ONLY;
    DuckDB db(file, &config);
    Connection con(db);
    auto result = con.Query(sql);
    getenv("NO_LIMIT") ? result->Print() : Printer::Print(result->ToBox(*con.context.get(), BoxRendererConfig({.max_rows = 200, .limit = 100})));
    return 0;
}
