#include "duckdb.hpp"
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>

#define CHECK(a) \
    if (a != 0)  \
    return a

class dbutil
{

private:
    duckdb::DuckDB *db = nullptr;
    duckdb::Connection *con = nullptr;
    duckdb::Appender *appender = nullptr;

public:
    dbutil()
    {
        this->db = new duckdb::DuckDB("ngx_log.db");
        this->con = new duckdb::Connection(*this->db);
        this->con->Query("DROP TABLE IF EXISTS nginx_log");
        this->con->Query("CREATE TABLE nginx_log ( time UINTEGER NOT NULL, remote_addr VARCHAR NOT NULL, remote_user VARCHAR NOT NULL, request VARCHAR NOT NULL, status USMALLINT NOT NULL, body_bytes_sent UINTEGER NOT NULL, http_referer VARCHAR NOT NULL, http_user_agent VARCHAR NOT NULL, http_x_forwarded_for VARCHAR NOT NULL, host VARCHAR NOT NULL, request_length UINTEGER NOT NULL, bytes_sent UINTEGER NOT NULL, upstream_addr VARCHAR NOT NULL, upstream_status USMALLINT NOT NULL, request_time REAL NOT NULL, upstream_response_time REAL NOT NULL, upstream_connect_time REAL NOT NULL, upstream_header_time REAL NOT NULL )");
    }
    ~dbutil()
    {
        if (this->appender)
        {
            delete this->appender;
            this->appender = nullptr;
        }
        if (this->con)
        {
            delete this->con;
            this->con = nullptr;
        }
        if (this->db)
        {
            delete this->db;
            this->db = nullptr;
        }
    }

    int begin()
    {
        this->appender = new duckdb::Appender(*this->con, "nginx_log");
        return 0;
    }

    // 如果出错将会抛出错误，否则必然返回0
    int insert_log(unsigned int time, const char *remote_addr, const char *remote_user, const char *request, unsigned int status, unsigned int body_bytes_sent, const char *http_referer, const char *http_user_agent, const char *http_x_forwarded_for, const char *host, unsigned int request_length, unsigned int bytes_sent, const char *upstream_addr, unsigned int upstream_status, float request_time, float upstream_response_time, float upstream_connect_time, float upstream_header_time)
    {
        this->appender->AppendRow(time, remote_addr, remote_user, request, status, body_bytes_sent, http_referer, http_user_agent, http_x_forwarded_for, host, request_length, bytes_sent, upstream_addr, upstream_status, request_time, upstream_response_time, upstream_connect_time, upstream_header_time);
        return 0;
    }

    int end()
    {
        if (this->appender)
        {
            this->appender->Close();
            delete this->appender;
            this->appender = nullptr;
        }
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
    duckdb::DBConfig config;
    config.options.access_mode = getenv("RW") ? duckdb::AccessMode::READ_WRITE : duckdb::AccessMode::READ_ONLY;
    duckdb::DuckDB db(file, &config);
    duckdb::Connection con(db);
    auto result = con.Query(sql);
    getenv("NO_LIMIT") ? result->Print() : duckdb::Printer::Print(result->ToBox(*con.context.get(), duckdb::BoxRendererConfig({.max_rows = 200, .limit = 100})));
    return 0;
}
