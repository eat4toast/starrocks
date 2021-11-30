// This file is made available under Elastic License 2.0.
// This file is based on code available under the Apache license here:
//   https://github.com/apache/incubator-doris/blob/master/be/src/olap/memtable.h

// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#ifndef STARROCKS_BE_SRC_OLAP_MEMTABLE_H
#define STARROCKS_BE_SRC_OLAP_MEMTABLE_H

#include <ostream>

#include "common/object_pool.h"
#include "runtime/mem_tracker.h"
#include "storage/olap_define.h"
#include "storage/skiplist.h"

namespace starrocks {

class ContiguousRow;
class RowsetWriter;
class Schema;
class SlotDescriptor;
class TabletSchema;
class Tuple;
class TupleDescriptor;

class MemTable {
public:
    MemTable(int64_t tablet_id, Schema* schema, const TabletSchema* tablet_schema,
             const std::vector<SlotDescriptor*>* slot_descs, TupleDescriptor* tuple_desc, KeysType keys_type,
             RowsetWriter* rowset_writer, MemTracker* mem_tracker);
    ~MemTable();

    int64_t tablet_id() const { return _tablet_id; }
    size_t memory_usage() const { return _mem_tracker->consumption(); }
    void insert(const Tuple* tuple);
    OLAPStatus flush();

private:
    class RowCursorComparator {
    public:
        RowCursorComparator(const Schema* schema);
        int operator()(const char* left, const char* right) const;

    private:
        const Schema* _schema;
    };
    typedef SkipList<char*, RowCursorComparator> Table;
    typedef Table::key_type TableKey;

    void _tuple_to_row(const Tuple* tuple, ContiguousRow* row, MemPool* mem_pool);
    void _aggregate_two_row(const ContiguousRow& new_row, TableKey row_in_skiplist);

    int64_t _tablet_id;
    Schema* _schema;
    const TabletSchema* _tablet_schema;
    TupleDescriptor* _tuple_desc;
    // the slot in _slot_descs are in order of tablet's schema
    const std::vector<SlotDescriptor*>* _slot_descs;
    KeysType _keys_type;

    RowCursorComparator _row_comparator;
    std::unique_ptr<MemTracker> _mem_tracker;
    // This is a buffer, to hold the memory referenced by the rows that have not
    // been inserted into the SkipList
    std::unique_ptr<MemPool> _buffer_mem_pool;
    // Only the rows will be inserted into SkipList can allocate memory from _table_mem_pool.
    // In this way, we can make MemTable::memory_usage() to be more accurate, and eventually
    // reduce the number of segment files that are generated by current load
    std::unique_ptr<MemPool> _table_mem_pool;
    ObjectPool _agg_object_pool;

    size_t _schema_size;
    Table* _skip_list;
    Table::Hint _hint;

    RowsetWriter* _rowset_writer;

}; // class MemTable

inline std::ostream& operator<<(std::ostream& os, const MemTable& table) {
    os << "MemTable(addr=" << &table << ", tablet=" << table.tablet_id() << ", mem=" << table.memory_usage();
    return os;
}

} // namespace starrocks

#endif // STARROCKS_BE_SRC_OLAP_MEMTABLE_H