#include "databases.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <bitset>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <exception>
#include <utility>

namespace fs = std::filesystem;
extern fs::path root;
extern uint32_t block_size;
extern std::string config_name,
            db_dir;

std::bitset<256> bit_blocks;

void db_full_write (Database& db)
{
    if (fs::exists(root/db_dir))
    {
        std::fstream file (root/db_dir/db.db_name, std::ios::binary | std::ios::out);
        if (file.is_open())
        {
            // first block is always 256 bytes
            // 4 bytes - num_of_block
            // db_name <= 248 !!!
            // 4 bytes for bl_size
            
            if (db.db_type != -1) // if not custom
            {
                // 0 - 255 bytes
                bool is_custom = 0;
                file.write(reinterpret_cast<char*>(&is_custom), 1); // not custom
                file.write(reinterpret_cast<char*>(&db.db_type), 1);
                file.write(reinterpret_cast<char*>(&block_size), 4); 
                file.write(db.db_name.c_str(), db.db_name.size());

                uint8_t pad_size = 250 - db.db_name.size();
                char * padding = new char [pad_size] ();
                file.write(padding, pad_size);
                delete [] padding;

                // bitset (256 bytes)
                char * bitset = new char [256] ();
                file.write(bitset, 256);
                delete [] bitset;

                for (int i = 0; i < db.tb_vec.size(); i++)
                {
                    char* membl = new char [128] ();
                    file.write(membl, 128);
                    delete [] membl;
                }

                // table meta block
                // size = block_size
                // max num of tables = 8 [0 - 7]
                // tb_name <= 64
                // col_name <= 32

                uint8_t num_of_bl = 0;
                for (auto& x : db.tb_vec)
                {
                    auto y = x->tuple_map.begin();
                    std::vector<uint8_t> pointers;
                    bool is_first = true;
                    while (y != x->tuple_map.end())
                    {
                        uint32_t pointer = 0,
                                 size = x->tuple_map.size();

                        file.write(reinterpret_cast<char*>(&x->table_type), 1);
                        if (is_first)
                            file.write(reinterpret_cast<char*>(&size), 4);
                        pointer += 5;

                        bit_blocks.set(num_of_bl);
                        pointers.push_back(num_of_bl + 1);
                        num_of_bl++;

                        uint32_t tup_size = y->second->size;
                        for (; pointer < block_size && y != x->tuple_map.end(); y++, pointer += (4 + tup_size))
                        {
                            uint32_t key = y->first;
                            file.write(reinterpret_cast<char*>(&key), 4);
                            y->second->write(file);
                        }

                        if (pointer < block_size)
                        {
                            char* membl = new char [block_size - pointer];
                            file.write(membl, block_size - pointer);
                            delete [] membl;
                        }
                        
                        is_first = false;
                    }

                    x->pointers = pointers;
                }

                file.seekp(256);
                file.write(bit_blocks.to_string().c_str(), 256);
                uint8_t num_of_iter = 0;
                for (auto& x : db.tb_vec)
                {
                    file.seekp(512 + 128 * num_of_iter);
                    num_of_iter++;

                    for (auto& y : x->pointers)
                    {
                        if (y != 0)
                            file.write(reinterpret_cast<char*>(&y), 1);
                        else 
                        {
                            break;
                        }
                    }
                }

            } else {

                file.write(reinterpret_cast<char*>(1), 1);
            }

            file.close();

        } else {
            std::cout << "Unable to create a database file\n";
        }

    } else {
        std::cout << "Unable to find directory with databases\nExiting ...\n";
    }
}

Database* db_meta_read (Database* db, std::string name) 
{
    if (fs::exists(root/db_dir/name))
    {
        std::fstream file (root/db_dir/name, std::ios::binary | std::ios::in);
        if (file.is_open())
        {
            bool is_custom = 0;           
            file.read(reinterpret_cast<char*>(&is_custom), 1);
            if (!is_custom) // if not custom
            {
                // meta block 
                int8_t type = -1;
                uint32_t bl_size = block_size;
                file.read(reinterpret_cast<char*>(&type), 1);
                file.read(reinterpret_cast<char*>(&bl_size), 4);

                char* name = new char [250] ();
                file.read(name, 250);

                switch(type)
                {
                    case 0 :
                        db = new Database(0);
                        break;

                    case 1 :
                        db = new Database(1, static_cast<std::string>(name));
                        break;

                    case 2 :
                        db = new Database(2, static_cast<std::string>(name));
                        break;

                    default:
                        throw std::invalid_argument("Can't read db. Unknown type");
                }

                delete [] name;
                db->bl_size = bl_size;
                block_size = bl_size;

                char* bits = new char [256];
                file.read(bits, 256);
                std::bitset<256> bitset2 {bits};
                bit_blocks |= bitset2;
                delete [] bits;

                // tables reading
                uint32_t num_of_tb = db->tb_vec.size();
                uint8_t num_of_iter = 0;
                for (auto& x : db->tb_vec)
                {
                    for (int i = 0; file.read(reinterpret_cast<char*>(&x->pointers[i]), 1) 
                            && i < 128 && x->pointers[i] != 0; i++);

                    if (x != db->tb_vec[db->tb_vec.size() - 1])
                    {
                        num_of_iter++;
                        file.seekg(512 + 128 * num_of_iter);
                    }
                }

                for (auto& x : db->tb_vec)
                {
                    for (int i = 0; x->pointers[i] != 0 && i < 128; i++)
                    {
                        file.seekg(512 + 128 * num_of_tb + bl_size * (x->pointers[i] - 1));
                        uint8_t tb_type = 255;
                        file.read(reinterpret_cast<char*>(&tb_type), 1);
                        if (tb_type != x->table_type)
                            throw std::invalid_argument("Error while reading data block");
                        uint32_t tuples = 0;
                        file.read(reinterpret_cast<char*>(&tuples), 4);
                        for (int i = 0; i < tuples; i++)
                        {
                            uint32_t key = 0;
                            file.read(reinterpret_cast<char*>(&key), 4);
                            tuple* tup = nullptr;
                            switch (tb_type)
                            {
                                case 0:
                                    tup = new dbases(file);
                                    break;

                                case 1:
                                    tup = new tb(file);
                                    break;

                                default:
                                    throw std::invalid_argument("Unknown table type");
                            }

                            auto pair = std::make_pair(key, tup);
                            x->tuple_map.insert(pair);
                        }
                    }
                }

            } else {

            }

            file.close();

        } else {
            std::cout << "Unable to open the database\nExiting ...\n";
        }

    } else {
        std::cout << "Unable to find directory with databases\nExiting ...\n";
    }

    return db;
}
