// This file is made available under Elastic License 2.0.
// This file is based on code available under the Apache license here:
//   https://github.com/apache/incubator-doris/blob/master/be/src/exec/local_file_writer.h

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

#pragma once

#include <cstdio>

#include "exec/file_writer.h"

namespace starrocks {

class RuntimeState;

class LocalFileWriter : public FileWriter {
public:
    LocalFileWriter(std::string path, int64_t start_offset);
    ~LocalFileWriter() override;

    Status open() override;

    Status write(const uint8_t* buf, size_t buf_len, size_t* written_len) override;

    Status close() override;

private:
    std::string _path;
    int64_t _start_offset;
    FILE* _fp;
};

} // end namespace starrocks
