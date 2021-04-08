#ifndef TABLE_H
#define TABLE_H
#include <vector>
#include <string>
#include "tuple.h"
#include <unordered_map>
#include <initializer_list>
#include <stdexcept>
#include <ios>
#include <iomanip>
#include <iostream>
#include <tuple>
#include "t_print.h"

class Database;

// interface
class Table
{
protected:
    // Database Meta
    // num of data blocks

    // <= 64 bytes
    uint8_t table_type;
    std::string table_name;
    // <= 32 bytes
    std::vector <std::string> col_names;
    std::vector <uint8_t> pointers;
    std::unordered_map<uint32_t, tuple*> tuple_map;
    std::vector<std::string> data_types;
    uint32_t row_num = 0;

    Table () : pointers(128) {};

public:

    virtual void insert (std::vector<std::string>,
                         std::vector<uint32_t> ) = 0;

    void select (std::vector<uint8_t> col_num, int32_t for_id, 
            int8_t num_of_col, char sign, int64_t  cond) const
    {
        if (col_num[0] == 255 && col_num.size() == 1)
        {
        } else { /* special columns - not done yet */ }
    }


    friend void db_full_write (Database&);
    friend Database* db_meta_read (Database* db, std::string name);
    friend void show_databases ();

    virtual ~Table() {};
};

class DB_table : public Table
{
    // dbases

public: 

    DB_table ()
    {
        table_type = 0;
        table_name = "SCHEMATA";
        col_names = {"db_id", "schema_name", "schema_type"};
        data_types = {"str", "int"};
    }

    void insert(std::vector<std::string> l1,
                std::vector<uint32_t> l2) override
    {
        // schema_name, schema_type
        if (l1.size() != 1 || l2.size() != 1)
            throw std::invalid_argument ("DB_table");

        // CLEAN !!!
        dbases* ins = new dbases ( l1[0], l2[0]);
        tuple_map.insert({tuple_map.size(), ins });
        row_num ++;
    }
};

/*
class Tb_table : public Table
{
    //tb
    
public:

    Tb_table ()
    {
        table_type = 1;
        table_name = "TABLES";
        col_names = {"table_id", "table_name", "rows_num", "db_id"};
    }


    void insert(std::vector<std::string> l1,
                std::vector<uint32_t> l2) override
    {
        if (l1.size() != 1 || l2.size() != 2)
            throw std::invalid_argument ("Tb_table");;

        tb* table = new tb ( l1[0], l2[0], l2[1] );
        tuple_map.insert({tuple_map.size(), table});
        row_num ++;
    }
};
*/


class Fac_table : public Table
{
    //fac

public: 

    Fac_table ()
    {
        table_type = 2;
        table_name = "FACULTIES";
        col_names = {"fac_id", "fac_name", "nuc_name", "num_dep", "is_hybrid"};
        data_types = {"str", "str", "int", "int"};
    }

    void insert (std::vector<std::string> l1,
            std::vector<uint32_t> l2) override
    {
        if (l1.size() != 2 || l2.size() != 2)
            throw std::invalid_argument("Fac_table");
        fac* faculty = new fac(l1[0], l1[1], l2[0], l2[1]);
        tuple_map.insert({tuple_map.size(), faculty});
        row_num ++;
    }
};

class Dep_table : public Table
{
    // dep

public:

    Dep_table()
    {
        table_type = 3;
        table_name = "DEPARTMENTS";
        col_names = {"dep_id", "dep_name", "fac_id"};
        data_types = {"str", "int"};
    }

    void insert (std::vector<std::string> l1,
            std::vector<uint32_t> l2) override 
    {
        if (l1.size() != 1 || l2.size() != 1)
            throw std::invalid_argument("Dep_table");
        dep* department = new dep (l1[0], l2[0]);
        tuple_map.insert({tuple_map.size(), department});
        row_num++;
    }
};

class Borg_table : public Table
{
    // borg

public:

    Borg_table()
    {
        table_type = 4;
        table_name = "BASE_ORG";
        col_names = {"borg_id", "borg_name", "fac_id"};
        data_types = {"str", "int"};
    }
    
    void insert (std::vector<std::string> l1,
            std::vector<uint32_t> l2) override 
    {
        if (l1.size() != 1 || l2.size() != 1)
            throw std::invalid_argument("Borg_table");
        borg* bg = new borg (l1[0], l2[0]);
        tuple_map.insert({tuple_map.size(), bg});
        row_num++;
    }

};

// type 5 and 6
class Dis_table : public Table
{
    // dis
public:

    Dis_table(uint8_t type)
    {
        table_type = type;
        data_types = {"str", "int", "int"};

        if (type == 5)
        {
            table_name = "DISCIPLINES_DEP";
            col_names = {"dis_id", "dis_name", "num_teach", "dep_id"};
        } else if (type == 6) {
            table_name = "DISCIPLINES_BORG";
            col_names = {"dis_id", "dis_name", "num_teach", "borg_id"};
        }
    }

    void insert (std::vector<std::string> l1,
            std::vector<uint32_t> l2) override 
    {
        if (l1.size() != 1 || l2.size() != 2)
            throw std::invalid_argument("Dis_table");
        dis* discipline = new dis (l1[0], l2[0], l2[1]);
        tuple_map.insert({tuple_map.size(), discipline});
        row_num++;
    }
};

#endif 
