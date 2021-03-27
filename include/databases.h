#ifndef DATABASE_H
#define DATABASE_H
#include <vector>
#include "table.h"
#include "tuple.h"

extern uint32_t block_size;

class Database 
{
protected:

    std::string db_name;
    uint32_t bl_size = block_size;
    std::vector<Table*> tb_vec;
    // num of data blocks

public:
    virtual void insert (std::string, uint8_t) = 0;
    virtual void write () = 0;

    friend void db_write (Database&);
};

#include "fs_work.h" 

// Meta database
class All_db : public Database
{
private:
    DB_table* db_t;
    Tb_table* tb_t;

public:

    All_db ()
    {
        db_name = "schemata";
        db_t = new DB_table;
        tb_t = new Tb_table;
        tb_vec = {db_t, tb_t};
    }

    void insert (std::string data, uint8_t tb_num) override
    {
        tb_vec[tb_num]->insert(data);
        // smth with tables_db
    }

    void write () override { db_write(*this); }

    ~All_db ()
    {
        delete db_t;
        delete tb_t;
    }

};

// Faculties
class Virtual_faculty : public Database
{
public:

    /*
    Virtual_faculty ()
    {
        Fac_table fac_t;
        Dep_table dep_t;
        Dis_table dis_dep_t;
        tb_vec = {fac_t, dep_t, dis_dep_t};
    }
    */
};

class Common_faculty : public Virtual_faculty
{

};

class Special_faculty : public Virtual_faculty
{
public:
    //Borg_table borg_t;
    //Dis_table dis_borg_t;
    //std::vector<Table> tb_vec;
};

#endif