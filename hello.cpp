#include <iostream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

using json = nlohmann::json;

std::string getString(const nlohmann::json& j) {
    if (j.is_null()) {
        return "";
    }
    if (j.is_array()) {
        std::string result;
        for (size_t i = 0; i < j.size(); ++i) {
            if (j[i].is_null()) continue;
            if (!result.empty()) result += ", ";
            result += j[i].get<std::string>();
        }
        return result;
    }
    return j.get<std::string>();
}

std::string getField(const nlohmann::json& a, const std::string& key) {
    if (!a.contains(key)) return "";
    return getString(a[key]);
}

bool insertField(sqlite3_stmt* stmt, int index, const nlohmann::json& a, const std::string& key) {
    return sqlite3_bind_text(stmt, index, getField(a, key).c_str(), -1, SQLITE_TRANSIENT) == 0;
}

bool insertArticle(sqlite3* db, const json& a) {
    const char* sql = R"(
        INSERT INTO article_table 
        (title, description, link, article_id, category, creator, pubDate, keywords)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::cout << "Inserting article: " << a["title"].get<std::string>() << std::endl;

    insertField(stmt, 1, a, "title");
    insertField(stmt, 2, a, "description");
    insertField(stmt, 3, a, "link");
    insertField(stmt, 4, a, "article_id");
    insertField(stmt, 5, a, "category");
    insertField(stmt, 6, a, "creator");
    insertField(stmt, 7, a, "pubDate");
    insertField(stmt, 8, a, "keywords");

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) {
        std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

int main() {
    sqlite3 *db;
    int rc = sqlite3_open("news.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return rc;
    } else {
        std::cout << "Opened database successfully" << std::endl;
    }

    std::string create_table_cmd = R"(
        CREATE TABLE IF NOT EXISTS article_table (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        title TEXT NOT NULL,
        description TEXT NOT NULL,
        link TEXT NOT NULL,
        article_id TEXT NOT NULL,
        category TEXT NOT NULL,
        creator TEXT NOT NULL,
        pubDate TEXT NOT NULL,
        keywords TEXT NOT NULL,
        UNIQUE(article_id)
        );
    )";

    char *err_msg = nullptr;
    rc = sqlite3_exec(db, create_table_cmd.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return rc;
    } else {
        std::cout << "Table created successfully" << std::endl;
    }

    std::string page = "";
    for (int i = 0; i < 20; ++i) {
        cpr::Response r = cpr::Get(cpr::Url{"https://newsdata.io/api/1/latest?country=us&language=en&datatype=news&removeDuplicate=1" + page}, 
            cpr::Header{{"X-ACCESS-KEY", "YOUR_API"}});

        json data = json::parse(r.text);
        for (auto a : data["results"]) {
            insertArticle(db, a);
        };

        page = "&page=" + data["nextPage"].get<std::string>();
    }

    sqlite3_close(db);
}